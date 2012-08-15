//
//  KRMaterialManager.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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
                        
                        if(cSymbols > 0) {
                            
                            if(strcmp(szSymbol[0], "newmtl") == 0 && cSymbols >= 2) {
                                
                                pMaterial = new KRMaterial(szSymbol[1]);
                                m_materials[szSymbol[1]] = pMaterial;
                            }
                            if(pMaterial != NULL) {
                                if(strcmp(szSymbol[0], "alpha_mode") == 0) {
                                    if(cSymbols == 2) {
                                        pMaterial->setAlphaTest(strcmp(szSymbol[1], "test") == 0);
                                    }
                                } else if(strcmp(szSymbol[0], "Ka") == 0) {
                                    char *pScan2 = szSymbol[1];
                                    double r = strtof(pScan2, &pScan2);
                                    if(cSymbols == 2) {
                                        pMaterial->setAmbient(KRVector3(r, r, r));
                                    } else if(cSymbols == 4) {
                                        pScan2 = szSymbol[2];
                                        double g = strtof(pScan2, &pScan2);
                                        pScan2 = szSymbol[3];
                                        double b = strtof(pScan2, &pScan2);
                                        pMaterial->setAmbient(KRVector3(r, g, b));
                                    }
                                } else if(strcmp(szSymbol[0], "Kd") == 0) {
                                    char *pScan2 = szSymbol[1];
                                    double r = strtof(pScan2, &pScan2);
                                    if(cSymbols == 2) {
                                        pMaterial->setDiffuse(KRVector3(r, r, r));
                                    } else if(cSymbols == 4) {
                                        pScan2 = szSymbol[2];
                                        double g = strtof(pScan2, &pScan2);
                                        pScan2 = szSymbol[3];
                                        double b = strtof(pScan2, &pScan2);
                                        pMaterial->setDiffuse(KRVector3(r, g, b));
                                    }
                                } else if(strcmp(szSymbol[0], "Ks") == 0) {
                                    char *pScan2 = szSymbol[1];
                                    double r = strtof(pScan2, &pScan2);
                                    if(cSymbols == 2) {
                                        pMaterial->setSpecular(KRVector3(r, r, r));
                                    } else if(cSymbols == 4) {
                                        pScan2 = szSymbol[2];
                                        double g = strtof(pScan2, &pScan2);
                                        pScan2 = szSymbol[3];
                                        double b = strtof(pScan2, &pScan2);
                                        pMaterial->setSpecular(KRVector3(r, g, b));
                                    }
                                } else if(strcmp(szSymbol[0], "Kr") == 0) {
                                    char *pScan2 = szSymbol[1];
                                    double r = strtof(pScan2, &pScan2);
                                    if(cSymbols == 2) {
                                        pMaterial->setReflection(KRVector3(r, r, r));
                                    } else if(cSymbols == 4) {
                                        pScan2 = szSymbol[2];
                                        double g = strtof(pScan2, &pScan2);
                                        pScan2 = szSymbol[3];
                                        double b = strtof(pScan2, &pScan2);
                                        pMaterial->setReflection(KRVector3(r, g, b));
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

                                    KRVector2 texture_scale = KRVector2(1.0f, 1.0f);
                                    KRVector2 texture_offset = KRVector2(0.0f, 0.0f);
                                    
                                    int iScanSymbol = 2;
                                    int iScaleParam = -1;
                                    int iOffsetParam = -1;
                                    while(iScanSymbol < cSymbols) {
                                        if(strcmp(szSymbol[iScanSymbol], "-s") == 0) {
                                            // Scale
                                            iScaleParam = 0;
                                            iOffsetParam = -1;
                                        } else if(strcmp(szSymbol[iScanSymbol], "-o") == 0) {
                                            // Offset
                                            iOffsetParam = 0;
                                            iScaleParam = -1;
                                        } else {
                                            char *pScan3 = szSymbol[iScanSymbol];
                                            double v = strtof(pScan3, &pScan3);
                                            if(iScaleParam == 0) {
                                                texture_scale.x = v;
                                                iScaleParam++;
                                            } else if(iScaleParam == 1) {
                                                texture_scale.y = v;
                                                iScaleParam++;
                                            } else if(iOffsetParam == 0) {
                                                texture_offset.x = v;
                                                iOffsetParam++;
                                            } else if(iOffsetParam == 1) {
                                                texture_offset.y = v;
                                                iOffsetParam++;
                                            }
                                        }
                                        iScanSymbol++;
                                    }

                                    if(strcmp(szSymbol[0], "map_Ka") == 0) {
                                        pMaterial->setAmbientMap(szSymbol[1], texture_scale, texture_offset);
                                    } else if(strcmp(szSymbol[0], "map_Kd") == 0) {
                                        pMaterial->setDiffuseMap(szSymbol[1], texture_scale, texture_offset);
                                    } else if(strcmp(szSymbol[0], "map_Ks") == 0) {
                                        pMaterial->setSpecularMap(szSymbol[1], texture_scale, texture_offset);
                                    } else if(strcmp(szSymbol[0], "map_Normal") == 0) {
                                        pMaterial->setNormalMap(szSymbol[1], texture_scale, texture_offset);
                                    } else if(strcmp(szSymbol[0], "map_Reflection") == 0) {
                                        pMaterial->setReflectionMap(szSymbol[1], texture_scale, texture_offset);
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
