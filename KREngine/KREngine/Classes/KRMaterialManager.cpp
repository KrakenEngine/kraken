//
//  KRMaterialManager.cpp
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-24.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#include "KRMaterialManager.h"

#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


KRMaterialManager::KRMaterialManager(KRTextureManager *pTextureManager, KRShaderManager *pShaderManager) {
    m_pTextureManager = pTextureManager;
    m_pShaderManager = pShaderManager;
}

KRMaterialManager::~KRMaterialManager() {
    
}


KRMaterial *KRMaterialManager::getMaterial(const char *szName) {
    map<std::string, KRMaterial *>::iterator itr = m_materials.find(szName);
    if(itr == m_materials.end()) {
        // Not found
        return NULL;
    } else {
        return (*itr).second;
    }
}

bool KRMaterialManager::loadFile(const char *szPath) {
    bool bSuccess = false;
    
    int fdFile = 0;
    int fileSize = 0;
    void *pFile = NULL;
    KRMaterial *pMaterial = NULL;
    char szSymbol[16][64];
    
    struct stat statbuf;
    fdFile = open(szPath, O_RDONLY);
    if(fdFile >= 0) {
        if(fstat(fdFile, &statbuf) >= 0) {
            if ((pFile = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdFile, 0)) == (caddr_t) -1) {
            } else {
                fileSize = statbuf.st_size;
                
                char *pScan = (char *)pFile;
                char *pEnd = (char *)pFile + fileSize;
                while(pScan < pEnd) {
                
                    // Scan through whitespace
                    while(pScan < pEnd && (*pScan == ' ' || *pScan == '\t' || *pScan == '\r' || *pScan == '\n')) {
                        pScan++;
                    }
                    
                    if(*pScan == '#') {
                        // Line is a comment line
                        
                        // Scan to the end of the line
                        while(pScan < pEnd && *pScan != '\r' && *pScan != '\n') {
                            pScan++;
                        }
                    } else {
                        int cSymbols = 0;
                        while(pScan < pEnd && *pScan != '\n' && *pScan != '\r') {
                            
                            char *pDest = szSymbol[cSymbols++];
                            while(pScan < pEnd && *pScan != ' ' && *pScan != '\n' && *pScan != '\r') {
                                *pDest++ = *pScan++;
                            }
                            *pDest = '\0';
                            
                            // Scan through whitespace, but don't advance to next line
                            while(pScan < pEnd && (*pScan == ' ' || *pScan == '\t')) {
                                pScan++;
                            }
                        }
                        
                        if(strcmp(szSymbol[0], "newmtl") == 0 && cSymbols >= 2) {
                            
                            pMaterial = new KRMaterial(m_pShaderManager);
                            m_materials[szSymbol[1]] = pMaterial;
                        } if(pMaterial != NULL) {
                            if(strcmp(szSymbol[0], "Ka") == 0) {
                                char *pScan2 = szSymbol[1];
                                double r = strtof(pScan2, &pScan2);
                                if(cSymbols == 2) {
                                    pMaterial->setAmbient(r, r, r);
                                } else if(cSymbols == 4) {
                                    pScan2 = szSymbol[2];
                                    double g = strtof(pScan2, &pScan2);
                                    pScan2 = szSymbol[3];
                                    double b = strtof(pScan2, &pScan2);
                                    pMaterial->setAmbient(r, g, b);
                                }
                            } else if(strcmp(szSymbol[0], "Kd") == 0) {
                                char *pScan2 = szSymbol[1];
                                double r = strtof(pScan2, &pScan2);
                                if(cSymbols == 2) {
                                    pMaterial->setDiffuse(r, r, r);
                                } else if(cSymbols == 4) {
                                    pScan2 = szSymbol[2];
                                    double g = strtof(pScan2, &pScan2);
                                    pScan2 = szSymbol[3];
                                    double b = strtof(pScan2, &pScan2);
                                    pMaterial->setDiffuse(r, g, b);
                                }
                            } else if(strcmp(szSymbol[0], "Ks") == 0) {
                                char *pScan2 = szSymbol[1];
                                double r = strtof(pScan2, &pScan2);
                                if(cSymbols == 2) {
                                    pMaterial->setSpecular(r, r, r);
                                } else if(cSymbols == 4) {
                                    pScan2 = szSymbol[2];
                                    double g = strtof(pScan2, &pScan2);
                                    pScan2 = szSymbol[3];
                                    double b = strtof(pScan2, &pScan2);
                                    pMaterial->setSpecular(r, g, b);
                                }
                            } else if(strcmp(szSymbol[0], "Tr") == 0) {
                                char *pScan2 = szSymbol[1];
                                double a = strtof(pScan2, &pScan2);
                                pMaterial->setTransparency(a);
                            } else if(strcmp(szSymbol[0], "Ns") == 0) {
                                char *pScan2 = szSymbol[1];
                                double a = strtof(pScan2, &pScan2);
                                pMaterial->setShininess(a);
                            } else if(strncmp(szSymbol[0], "map", 3) == 0) {
                                // Truncate file extension
                                char *pScan2 = szSymbol[1];
                                char *pLastPeriod = NULL;
                                while(*pScan2 != '\0') {
                                    if(*pScan2 == '.') {
                                        pLastPeriod = pScan2;
                                    }
                                    pScan2++;
                                }
                                if(pLastPeriod) {
                                    *pLastPeriod = '\0';
                                }

                                KRTexture *pTexture = m_pTextureManager->getTexture(szSymbol[1]);
                                if(pTexture) {
                                    if(strcmp(szSymbol[0], "map_Ka") == 0) {
                                        pMaterial->setAmbientMap(pTexture);
                                    } else if(strcmp(szSymbol[0], "map_Kd") == 0) {
                                        pMaterial->setDiffuseMap(pTexture);
                                    } else if(strcmp(szSymbol[0], "map_Ks") == 0) {
                                        pMaterial->setSpecularMap(pTexture);
                                    } else if(strcmp(szSymbol[0], "map_Normal") == 0) {
                                        pMaterial->setNormalMap(pTexture);
                                    }
                                }
                            }
                        }

                    }

                }
                
                bSuccess = true;
            }
        }
    }
    
    if(pFile != NULL) {
        munmap(pFile, fileSize);
    }
    
    if(fdFile != 0) {
        close(fdFile);
    }
    
    return bSuccess;
}
