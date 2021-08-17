//
//  KRDeviceManager.cpp
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KRDeviceManager.h"

KRDeviceManager::KRDeviceManager(KRContext& context)
  : KRContextObject(context)
  , m_vulkanInstance(VK_NULL_HANDLE)
  , m_topDeviceHandle(0)
{

}

KRDeviceManager::~KRDeviceManager()
{
  destroyDevices();
  if (m_vulkanInstance != VK_NULL_HANDLE) {
    vkDestroyInstance(m_vulkanInstance, NULL);
    m_vulkanInstance = VK_NULL_HANDLE;
  }
}

bool KRDeviceManager::haveVulkan() const
{
  return m_vulkanInstance != VK_NULL_HANDLE;
}

bool KRDeviceManager::haveDevice() const
{
  return !m_devices.empty();
}

void 
KRDeviceManager::destroyDevices()
{
  const std::lock_guard<std::mutex> lock(KRContext::g_DeviceInfoMutex);
  m_devices.clear();
}

void
KRDeviceManager::initialize()
{
  VkResult res = volkInitialize();
  if (res != VK_SUCCESS) {
    destroyDevices();
    return;
  }

  // initialize the VkApplicationInfo structure
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = "Test"; // TODO - Change Me!
  app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  app_info.pEngineName = "Kraken Engine";
  app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  // VK_KHR_surface and VK_KHR_win32_surface

  char* extensions[] = {
    "VK_KHR_surface",
#ifdef WIN32
    "VK_KHR_win32_surface",
#endif
  };

  // initialize the VkInstanceCreateInfo structure
  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
#ifdef WIN32
  inst_info.enabledExtensionCount = 2;
#else
  inst_info.enabledExtensionCount = 1;
#endif
  inst_info.ppEnabledExtensionNames = extensions;
  inst_info.enabledLayerCount = 0;
  inst_info.ppEnabledLayerNames = NULL;

  res = vkCreateInstance(&inst_info, NULL, &m_vulkanInstance);
  if (res != VK_SUCCESS) {
    destroyDevices();
    return;
  }

  volkLoadInstance(m_vulkanInstance);

  createDevices();
}

void KRDeviceManager::createDevices()
{
  const std::lock_guard<std::mutex> deviceLock(KRContext::g_DeviceInfoMutex);
  if (m_devices.size() > 0) {
    return;
  }
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    return;
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, physicalDevices.data());

  const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  std::vector<std::unique_ptr<KRDevice>> candidateDevices;

  for (const VkPhysicalDevice& physicalDevice : physicalDevices) {
    std::unique_ptr<KRDevice> device = std::make_unique<KRDevice>(physicalDevice);
    if (!device->initialize(deviceExtensions)) {
      continue;
    }

    bool addDevice = false;
    if (candidateDevices.empty()) {
      addDevice = true;
    }
    else {
      VkPhysicalDeviceType collectedType = candidateDevices[0]->m_deviceProperties.deviceType;
      if (collectedType == device->m_deviceProperties.deviceType) {
        addDevice = true;
      }
      else if (device->m_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        // Discrete GPU's are always the best choice
        candidateDevices.clear();
        addDevice = true;
      }
      else if (collectedType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device->m_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        // Integrated GPU's are the second best choice
        candidateDevices.clear();
        addDevice = true;
      }
      else if (collectedType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && collectedType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && device->m_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
        // Virtual GPU's are the 3rd best choice
        candidateDevices.clear();
        addDevice = true;
      }
    }
    if (addDevice) {
      candidateDevices.push_back(std::move(device));
    }
  }

  for (auto itr = candidateDevices.begin(); itr != candidateDevices.end(); itr++) {
    std::unique_ptr<KRDevice> device = std::move(*itr);
    m_devices[++m_topDeviceHandle] = std::move(device);
  }
}

KRDevice& KRDeviceManager::getDeviceInfo(KrDeviceHandle handle)
{
  return *m_devices[handle];
}

VkInstance& KRDeviceManager::getVulkanInstance()
{
  return m_vulkanInstance;
}

KrSurfaceHandle KRDeviceManager::getBestDeviceForSurface(const VkSurfaceKHR& surface)
{
  KrDeviceHandle deviceHandle = 0;
  for (auto itr = m_devices.begin(); itr != m_devices.end(); itr++) {
    KRDevice& device = *(*itr).second;
    VkBool32 canPresent = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device.m_device, device.m_graphicsFamilyQueueIndex, surface, &canPresent);
    if (canPresent) {
      deviceHandle = (*itr).first;
      break;
    }
  }
  return deviceHandle;
}