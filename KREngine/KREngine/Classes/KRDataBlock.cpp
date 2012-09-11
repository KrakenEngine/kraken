//
//  KRDataBlock.cpp
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

#include <iostream>

#include "KRDataBlock.h"
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

KRDataBlock::KRDataBlock() {
    m_data = NULL;
    m_data_size = 0;
    m_fdPackFile = 0;
    m_bMalloced = false;
}

KRDataBlock::~KRDataBlock() {
    unload();
}

// Unload a file, releasing any mmap'ed file handles or malloc'ed ram that was in use
void KRDataBlock::unload()
{
    if(m_fdPackFile) {
        // Memory mapped file
        if(m_bMalloced) {
            munmap(m_data, m_data_size);
        }
        close(m_fdPackFile);
    } else if(m_data != NULL && m_bMalloced) {
        // Malloc'ed data
        free(m_data);
    }
    
    m_bMalloced = false;
    m_data = NULL;
    m_data_size = 0;
    m_fdPackFile = 0;
}

// Encapsulate a pointer.  Note - The pointer will not be free'ed
bool KRDataBlock::load(void *data, size_t size)
{
    unload();
    m_data = data;
    m_data_size = size;
    return true;
}

// Load a file into memory using mmap.  The data pointer will be protected as read-only until append() or expand() is called
bool KRDataBlock::load(const std::string &path)
{
    bool success = false;
    unload();
    
    struct stat statbuf;
    m_fdPackFile = open(path.c_str(), O_RDONLY);
    if(m_fdPackFile >= 0) {
        if(fstat(m_fdPackFile, &statbuf) >= 0) {
            if ((m_data = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, m_fdPackFile, 0)) == (caddr_t) -1) {
            } else {
                m_data_size = statbuf.st_size;
                success = true;
            }
        }
    }
    if(!success) {
        // If anything failed, don't leave the object in an invalid state
        unload();
    }
    return success;
}

// Return a pointer to the start of the data block
void *KRDataBlock::getStart() {
    return m_data;
}

// Return a pointer to the byte after the end of the data block
void *KRDataBlock::getEnd() {
    return (unsigned char *)m_data + m_data_size;
}

// Return the size of the data block.  Use append() or expand() to make the data block larger
size_t KRDataBlock::getSize() const {
    return m_data_size;
}

// Expand the data block, and switch it to read-write mode.  Note - this may result in a mmap'ed file being copied to malloc'ed ram and then closed
void KRDataBlock::expand(size_t size)
{
    if(m_data == NULL) {
        // Starting with an empty data block; allocate memory on the heap
        m_data = malloc(size);
        m_data_size = size;
        m_bMalloced = true;
    } else if(m_bMalloced) {
        // Starting with a malloc'ed data block; realloc it expand
        m_data = realloc(m_data, m_data_size + size);
        m_data_size += size;
    } else {
        // Starting with a mmap'ed data block; copy it to ram before expanding to avoid updating the original file until save() is called
        // ... Or starting with a pointer reference, we must make our own copy and must not free the pointer
        void *pNewData = malloc(m_data_size + size);
        memcpy((unsigned char *)pNewData, m_data, m_data_size); // Copy exising data
        // Unload existing data allocation, which is now redundant
        size_t new_size = m_data_size + size; // We need to store this before unload() as unload() will reset it
        unload();
        m_bMalloced = true;
        m_data = pNewData;
        m_data_size = new_size;
    }
}

// Append data to the end of the block, increasing the size of the block and making it read-write.
void KRDataBlock::append(void *data, size_t size) {
    // Expand the data block
    expand(size);
    
    // Fill the new space with the data to append
    memcpy((unsigned char *)m_data + m_data_size - size, data, size);
}

// Save the data to a file, and switch to read-only mode.  The data pointer will be replaced with a mmap'ed address of the file; the malloc'ed data will be freed
bool KRDataBlock::save(const std::string& path) {
    int fdNewFile = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if(fdNewFile == -1) {
        return false;
    } else {
        // Seek to end of file and write a byte to enlarge it
        lseek(fdNewFile, m_data_size-1, SEEK_SET);
        write(fdNewFile, "", 1);
        
        // Now map it...
        void *pNewData = mmap(0, m_data_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdNewFile, 0);
        if(pNewData == (caddr_t) -1) {
            close(fdNewFile);
            return false;
        } else if(m_data != NULL) {
            // Copy data to new file
            memcpy(pNewData, m_data, m_data_size);
            
            // Unload existing data allocation, which is now redundant
            size_t new_size = m_data_size; // We need to store this, as unload() will reset it
            unload();

            // Protect new mmap'ed memory
            mprotect(pNewData, m_data_size, PROT_READ);
            
            // Switch pointer to use new mmap'ed memory
            m_data_size = new_size;
            m_fdPackFile = fdNewFile;
            m_data = pNewData;
        }
        return true;
    }
}
