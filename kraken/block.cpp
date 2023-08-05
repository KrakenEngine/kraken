//
//  block.cpp
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
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

#include "block.h"
#include "KREngine-common.h"
#include "KRResource.h"
#include "KRContext.h"

#include <errno.h>
#if defined(__APPLE__) || defined(ANDROID)
#include <unistd.h>
#include <sys/mman.h>
#endif

#define KRENGINE_MIN_MMAP 32768
#define KRAKEN_MEM_ROUND_DOWN_PAGE(x) ((x) & ~(KRContext::KRENGINE_SYS_ALLOCATION_GRANULARITY - 1))
#define KRAKEN_MEM_ROUND_UP_PAGE(x) ((((x) - 1) & ~(KRContext::KRENGINE_SYS_ALLOCATION_GRANULARITY - 1)) + KRContext::KRENGINE_SYS_ALLOCATION_GRANULARITY)

int m_mapCount = 0;
size_t m_mapSize = 0;
size_t m_mapOverhead = 0;

namespace mimir {

Block::Block()
{
  m_data = NULL;
  m_data_size = 0;
  m_data_offset = 0;
#if defined(_WIN32) || defined(_WIN64)
  m_hPackFile = INVALID_HANDLE_VALUE;
  m_hFileMapping = NULL;
#elif defined(__APPLE__)
  m_fdPackFile = 0;
#endif
  m_fileName = "";
  m_mmapData = NULL;
  m_fileOwnerDataBlock = NULL;
  m_bMalloced = false;
  m_lockCount = 0;
  m_bReadOnly = false;
}

Block::Block(void* data, size_t size)
{
  m_data = NULL;
  m_data_size = 0;
  m_data_offset = 0;
#if defined(_WIN32) || defined(_WIN64)
  m_hPackFile = INVALID_HANDLE_VALUE;
  m_hFileMapping = NULL;
#elif defined(__APPLE__)
  m_fdPackFile = 0;
#endif
  m_fileName = "";
  m_mmapData = NULL;
  m_fileOwnerDataBlock = NULL;
  m_bMalloced = false;
  m_lockCount = 0;
  m_bReadOnly = false;
  load(data, size);
}

Block::~Block()
{
  unload();
}

// Unload a file, releasing any mmap'ed file handles or malloc'ed ram that was in use
void Block::unload()
{
  assert(m_lockCount == 0);

#if defined(_WIN32) || defined(_WIN64)
  if (m_hPackFile != INVALID_HANDLE_VALUE) {
    // Memory mapped file
    if (m_fileOwnerDataBlock == this) {
      CloseHandle(m_hPackFile);
    }
    m_hPackFile = INVALID_HANDLE_VALUE;
  }
#elif defined(__APPLE__)
  if (m_fdPackFile) {
    // Memory mapped file
    if (m_fileOwnerDataBlock == this) {
      close(m_fdPackFile);
    }
    m_fdPackFile = 0;
  }
#endif

  if (m_data != NULL && m_bMalloced) {
    // Malloc'ed data
    free(m_data);
  }

  m_bMalloced = false;
  m_data = NULL;
  m_data_size = 0;
  m_data_offset = 0;
  m_fileName = "";
  m_mmapData = NULL;
  m_fileOwnerDataBlock = NULL;
  m_bReadOnly = false;
}

// Encapsulate a pointer.  Note - The pointer will not be free'ed
bool Block::load(void* data, size_t size)
{
  unload();
  m_data = data;
  m_data_size = size;
  m_data_offset = 0;
  m_bReadOnly = false;
  return true;
}

// Load a file into memory using mmap.  The data pointer will be protected as read-only until append() or expand() is called
bool Block::load(const std::string& path)
{
  bool success = false;
  unload();

  m_bReadOnly = true;

#if defined(_WIN32) || defined(_WIN64)
  m_hPackFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (m_hPackFile != INVALID_HANDLE_VALUE) {
    m_fileOwnerDataBlock = this;
    m_fileName = KRResource::GetFileBase(path);
    FILE_STANDARD_INFO fileInfo;
    if (GetFileInformationByHandleEx(m_hPackFile, FileStandardInfo, &fileInfo, sizeof(fileInfo))) {
      m_data_size = fileInfo.EndOfFile.QuadPart;
      m_data_offset = 0;
      success = true;
    }
  }
#elif defined(__APPLE__)
  m_fdPackFile = open(path.c_str(), O_RDONLY);
  if (m_fdPackFile >= 0) {
    m_fileOwnerDataBlock = this;
    m_fileName = KRResource::GetFileBase(path);
    struct stat statbuf;
    if (fstat(m_fdPackFile, &statbuf) >= 0) {
      m_data_size = statbuf.st_size;
      m_data_offset = 0;
      success = true;
    }
  }
#endif
  if (!success) {
    // If anything failed, don't leave the object in an invalid state
    unload();
  }
  return success;
}

// Create a Block encapsulating a sub-region of this block.  The caller is responsible to free the object.
Block* Block::getSubBlock(int start, int length)
{
  Block* new_block = new Block();

  new_block->m_data_size = length;
#if defined(_WIN32) || defined(_WIN64)
  if (m_hPackFile != INVALID_HANDLE_VALUE) {
    new_block->m_hPackFile = m_hPackFile;
#elif defined(__APPLE__) || defined(ANDROID)
  if (m_fdPackFile) {
    new_block->m_fdPackFile = m_fdPackFile;
#else
#error Unsupported
#endif
  new_block->m_fileOwnerDataBlock = m_fileOwnerDataBlock;
  new_block->m_data_offset = start + m_data_offset;
  } else if (m_bMalloced) {
    new_block->m_data = (unsigned char*)m_data + start + m_data_offset;
  }
  new_block->m_bReadOnly = true;

  return new_block;
}

// Return a pointer to the start of the data block
void* Block::getStart()
{
  assertLocked();
  return m_data;
}

// Return a pointer to the byte after the end of the data block
void* Block::getEnd()
{
  assertLocked();
  return (unsigned char*)m_data + m_data_size;
}

// Return the size of the data block.  Use append() or expand() to make the data block larger
size_t Block::getSize() const
{
  return m_data_size;
}

// Expand the data block, and switch it to read-write mode.  Note - this may result in a mmap'ed file being copied to malloc'ed ram and then closed
void Block::expand(size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
  if (m_data == NULL && m_hPackFile == INVALID_HANDLE_VALUE) {
#elif defined(__APPLE__) || defined(ANDROID)
  if (m_data == NULL && m_fdPackFile == 0) {
#else
#error Unsupported
#endif
  // Starting with an empty data block; allocate memory on the heap
  m_data = malloc(size);
  assert(m_data != NULL);
  m_data_size = size;
  m_data_offset = 0;
  m_bMalloced = true;
  } else if (m_bMalloced) {
    // Starting with a malloc'ed data block; realloc it expand
    m_data = realloc(m_data, m_data_size + size);
    m_data_size += size;
  } else {
    // Starting with a mmap'ed data block, an encapsulated pointer, or a sub-block; copy it to ram before expanding to avoid updating the original file until save() is called
    // ... Or starting with a pointer reference, we must make our own copy and must not free the pointer
    void* pNewData = malloc(m_data_size + size);
    assert(pNewData != NULL);

    // Copy exising data
    copy(pNewData);

    // Unload existing data allocation, which is now redundant
    size_t new_size = m_data_size + size; // We need to store this before unload() as unload() will reset it
    unload();
    m_bMalloced = true;
    m_data = pNewData;
    m_data_size = new_size;
    m_data_offset = 0;
  }
}

// Append data to the end of the block, increasing the size of the block and making it read-write.
void Block::append(void* data, size_t size)
{
  // Expand the data block
  expand(size);

  // Fill the new space with the data to append
  lock();
  memcpy((unsigned char*)m_data + m_data_size - size, data, size);
  unlock();
}


// Copy the entire data block to the destination pointer
void Block::copy(void* dest)
{
  copy(dest, 0, (int)m_data_size);
}

// Copy a range of data to the destination pointer
void Block::copy(void* dest, int start, int count)
{
#if defined(_WIN32) || defined(_WIN64)
  if (m_lockCount == 0 && m_hPackFile != INVALID_HANDLE_VALUE) {
    // Optimization: If we haven't mmap'ed or malloced the data already, ReadFile() it directly from the file into the buffer
    LARGE_INTEGER distance;
    distance.QuadPart = start + m_data_offset;
    bool success = SetFilePointerEx(m_hPackFile, distance, NULL, FILE_BEGIN);
    assert(success);

    void* w = dest;
    DWORD bytes_remaining = count;
    while (bytes_remaining > 0) {
      DWORD bytes_read = 0;
      success = ReadFile(m_hPackFile, w, bytes_remaining, &bytes_read, NULL);
      assert(success);
      assert(bytes_read > 0);
      w = (unsigned char*)w + bytes_read;
      bytes_remaining -= bytes_read;
    }
    assert(bytes_remaining == 0);
#elif defined(__APPLE__) || defined(ANDROID)
  if (m_lockCount == 0 && m_fdPackFile != 0) {
    // Optimization: If we haven't mmap'ed or malloced the data already, pread() it directly from the file into the buffer
    ssize_t r = pread(m_fdPackFile, dest, count, start + m_data_offset);
    assert(r != -1);
#else
#error Unsupported
#endif
  } else {
  lock();
  memcpy((unsigned char*)dest, (unsigned char*)m_data + start, count);
  unlock();
}
}

// Append data to the end of the block, increasing the size of the block and making it read-write.
void Block::append(Block & data)
{
  data.lock();
  append(data.getStart(), data.getSize());
  data.unlock();
}

// Append string to the end of the block, increasing the size of the block and making it read-write.  The resulting datablock includes a terminating character
void Block::append(const std::string & s)
{
  const char* szText = s.c_str();
  size_t text_length = strlen(szText);
  size_t prev_size = getSize();
  if (prev_size == 0) {
    // First string appended to data block, just memcpy it..
    append((void*)szText, text_length + 1);
  } else {
    // prev_size includes a null terminating character, don't need to add two.
    expand(text_length);
    lock();
    // Copy new string, overwriting prior null terminating character and
    // including new terminating character
    memcpy((unsigned char*)m_data + prev_size - 1, szText, text_length + 1);
    unlock();
  }
}

// Save the data to a file.
bool Block::save(const std::string & path)
{
#if defined(_WIN32) || defined(_WIN64)
  bool success = true;
  HANDLE hNewFile = INVALID_HANDLE_VALUE;
  HANDLE hFileMapping = NULL;
  void* pNewData = NULL;

  hNewFile = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hNewFile == INVALID_HANDLE_VALUE) {
    success = false;
  }

  if (success) {
    hFileMapping = CreateFileMappingFromApp(hNewFile, NULL, PAGE_READWRITE, m_data_size, NULL);
    if (hFileMapping == NULL) {
      success = false;
    }
  }

  if (success) {
    pNewData = MapViewOfFileFromApp(hFileMapping, FILE_MAP_WRITE, 0, m_data_size);
    if (pNewData == NULL) {
      success = false;
    }
  }

  if (success) {
    // Copy data to new file
    copy(pNewData);
  }

  if (pNewData != NULL) {
    UnmapViewOfFile(pNewData);
  }

  if (hFileMapping != NULL) {
    CloseHandle(hFileMapping);
  }

  if (hNewFile != INVALID_HANDLE_VALUE) {
    CloseHandle(hNewFile);
  }

  return success;

#elif defined(__APPLE__) || defined(ANDROID)
  int fdNewFile = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
  if (fdNewFile == -1) {
    return false;
  }

  // Seek to end of file and write a byte to enlarge it
  lseek(fdNewFile, m_data_size - 1, SEEK_SET);
  write(fdNewFile, "", 1);

  // Now map it...
  void* pNewData = mmap(0, m_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdNewFile, 0);
  if (pNewData == (caddr_t)-1) {
    close(fdNewFile);
    return false;
  }
  if (m_data != NULL) {
    // Copy data to new file
    copy(pNewData);

    // Unmap the new file
    munmap(pNewData, m_data_size);

    // Close the new file
    close(fdNewFile);
  }
  return true;

#else
#error Unsupported
#endif
}

// Get contents as a string
std::string Block::getString()
{
  Block b;
  b.append(*this);
  b.append((void*)"\0", 1); // Ensure data is null terminated, to read as a string safely
  b.lock();
  std::string ret = std::string((char*)b.getStart());
  b.unlock();
  return ret;
}

#if defined(_WIN32) || defined(_WIN64)
void ReportWindowsLastError(LPCTSTR lpszFunction)
{
  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&lpMsgBuf,
    0, NULL);

  // Display the error message and exit the process

  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
  fprintf(stderr,
    TEXT("%s failed with error %d: %s\n"),
    lpszFunction, dw, (LPCTSTR)lpMsgBuf);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}
#endif

// Lock the memory, forcing it to be loaded into a contiguous block of address space
void Block::lock()
{
  if (m_lockCount == 0) {

    // Memory mapped file; ensure data is mapped to ram
#if defined(_WIN32) || defined(_WIN64)
    if (m_hPackFile != INVALID_HANDLE_VALUE) {
#elif defined(__APPLE__) || defined(ANDROID)
    if (m_fdPackFile) {
#else
#error Unsupported
#endif
    if (m_data_size < KRENGINE_MIN_MMAP) {
      m_data = malloc(m_data_size);
      assert(m_data != NULL);
      copy(m_data);
    } else {
      size_t alignment_offset = m_data_offset & (KRContext::KRENGINE_SYS_ALLOCATION_GRANULARITY - 1);
      assert(m_mmapData == NULL);
#if defined(_WIN32) || defined(_WIN64)
      m_hFileMapping = CreateFileMappingFromApp(m_hPackFile, NULL, m_bReadOnly ? PAGE_READONLY : PAGE_READWRITE, m_fileOwnerDataBlock->getSize(), NULL);
      if (m_hFileMapping == NULL) {
        ReportWindowsLastError("CreateFileMappingFromApp");
      }
      assert(m_hFileMapping != NULL);

      m_mmapData = MapViewOfFileFromApp(m_hFileMapping, m_bReadOnly ? FILE_MAP_READ | FILE_MAP_COPY : FILE_MAP_WRITE, m_data_offset - alignment_offset, m_data_size + alignment_offset);
      if (m_mmapData == NULL) {
        ReportWindowsLastError("MapViewOfFileFromApp");
      }
      assert(m_mmapData != NULL);
#elif defined(__APPLE__) || defined(ANDROID)
      //fprintf(stderr, "Block::lock - \"%s\" (%i)\n", m_fileOwnerDataBlock->m_fileName.c_str(), m_lockCount);
      // Round m_data_offset down to the next memory page, as required by mmap

      if ((m_mmapData = mmap(0, m_data_size + alignment_offset, m_bReadOnly ? PROT_READ : PROT_WRITE, MAP_SHARED, m_fdPackFile, m_data_offset - alignment_offset)) == (caddr_t)-1) {
        int iError = errno;
        switch (iError) {
        case EACCES:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with EACCES");
          break;
        case EBADF:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with EBADF");
          break;
        case EMFILE:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with EMFILE");
          break;
        case EINVAL:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with EINVAL");
          break;
        case ENOMEM:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with ENOMEM");
          break;
        case ENXIO:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with ENXIO");
          break;
        case EOVERFLOW:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with EOVERFLOW");
          break;
        default:
          KRContext::Log(KRContext::LOG_LEVEL_ERROR, "mmap failed with errno: %i", iError);
          break;
        }
        assert(false); // mmap() failed.
      }
#else
#error Unsupported
#endif
      m_mapCount++;
      m_mapSize += m_data_size;
      m_mapOverhead += alignment_offset + KRAKEN_MEM_ROUND_UP_PAGE(m_data_size + alignment_offset) - m_data_size + alignment_offset;
      // fprintf(stderr, "Mapped: %i Size: %d Overhead: %d\n", m_mapCount, m_mapSize, m_mapOverhead);
      m_data = (unsigned char*)m_mmapData + alignment_offset;
    }
    }
  }
m_lockCount++;
}

// Unlock the memory, releasing the address space for use by other allocations
void Block::unlock()
{
  // We expect that the data block was previously locked
  assertLocked();


  if (m_lockCount == 1) {

    // Memory mapped file; ensure data is unmapped from ram
#if defined(_WIN32) || defined(_WIN64)
    if (m_hPackFile != INVALID_HANDLE_VALUE) {
#elif defined(__APPLE__) || defined(ANDROID)
    if (m_fdPackFile) {
#else
#error Undefined
#endif
    if (m_data_size < KRENGINE_MIN_MMAP) {
      free(m_data);
      m_data = NULL;
    } else {
      //fprintf(stderr, "Block::unlock - \"%s\" (%i)\n", m_fileOwnerDataBlock->m_fileName.c_str(), m_lockCount);
#if defined(_WIN32) || defined(_WIN64)
      if (m_mmapData != NULL) {
        UnmapViewOfFile(m_mmapData);
      }
      if (m_hFileMapping != NULL) {
        CloseHandle(m_hFileMapping);
        m_hFileMapping = NULL;
      }
#elif defined(__APPLE__) || defined(ANDROID)
      munmap(m_mmapData, m_data_size);
#else
#error Undefined
#endif
      m_data = NULL;
      m_mmapData = NULL;
      m_mapCount--;
      m_mapSize -= m_data_size;
      size_t alignment_offset = m_data_offset & (KRContext::KRENGINE_SYS_ALLOCATION_GRANULARITY - 1);
      m_mapOverhead -= alignment_offset + KRAKEN_MEM_ROUND_UP_PAGE(m_data_size + alignment_offset) - m_data_size + alignment_offset;
      // fprintf(stderr, "Mapped: %i Size: %d Overhead: %d\n", m_mapCount, m_mapSize, m_mapOverhead);
    }
    }
  }
m_lockCount--;
}

// Assert if not locked
void Block::assertLocked()
{
  assert(m_lockCount > 0);
}

} // namespace mimir
