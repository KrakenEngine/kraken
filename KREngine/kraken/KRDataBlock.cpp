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

#include "KRDataBlock.h"
#include "KREngine-common.h"
#include "KRResource.h"

#include <errno.h>

int m_mapCount = 0;
size_t m_mapSize = 0;
size_t m_mapOverhead = 0;

KRDataBlock::KRDataBlock() {
    m_data = NULL;
    m_data_size = 0;
    m_data_offset = 0;
    m_fdPackFile = 0;
    m_fileName = "";
    m_mmapData = NULL;
    m_fileOwnerDataBlock = NULL;
    m_bMalloced = false;
    m_lockCount = 0;
    m_bReadOnly = false;
}

KRDataBlock::KRDataBlock(void *data, size_t size) {
    m_data = NULL;
    m_data_size = 0;
    m_data_offset = 0;
    m_fdPackFile = 0;
    m_fileName = "";
    m_mmapData = NULL;
    m_fileOwnerDataBlock = NULL;
    m_bMalloced = false;
    m_lockCount = 0;
    m_bReadOnly = false;
    load(data, size);
}

KRDataBlock::~KRDataBlock() {
    unload();
}

// Unload a file, releasing any mmap'ed file handles or malloc'ed ram that was in use
void KRDataBlock::unload()
{
    assert(m_lockCount == 0);
    
    if(m_fdPackFile) {
        // Memory mapped file
        if(m_fileOwnerDataBlock == this) {
            close(m_fdPackFile);
        }
    } else if(m_data != NULL && m_bMalloced) {
        // Malloc'ed data
        free(m_data);
    }
    
    m_bMalloced = false;
    m_data = NULL;
    m_data_size = 0;
    m_data_offset = 0;
    m_fdPackFile = 0;
    m_fileName = "";
    m_mmapData = NULL;
    m_fileOwnerDataBlock = NULL;
    m_bReadOnly = false;
}

// Encapsulate a pointer.  Note - The pointer will not be free'ed
bool KRDataBlock::load(void *data, size_t size)
{
    unload();
    m_data = data;
    m_data_size = size;
    m_data_offset = 0;
    m_bReadOnly = false;
    return true;
}

// Load a file into memory using mmap.  The data pointer will be protected as read-only until append() or expand() is called
bool KRDataBlock::load(const std::string &path)
{
    bool success = false;
    unload();
    
    struct stat statbuf;
    m_bReadOnly = true;
    m_fdPackFile = open(path.c_str(), O_RDONLY);
    if(m_fdPackFile >= 0) {
        m_fileOwnerDataBlock = this;
        m_fileName = KRResource::GetFileBase(path);
        if(fstat(m_fdPackFile, &statbuf) >= 0) {
            m_data_size = statbuf.st_size;
            m_data_offset = 0;
            success = true;
        }
    }
    if(!success) {
        // If anything failed, don't leave the object in an invalid state
        unload();
    }
    return success;
}

// Create a KRDataBlock encapsulating a sub-region of this block.  The caller is responsible to free the object.
KRDataBlock *KRDataBlock::getSubBlock(int start, int length)
{
    KRDataBlock *new_block = new KRDataBlock();

    new_block->m_data_size = length;
    if(m_fdPackFile) {
        new_block->m_fdPackFile = m_fdPackFile;
        new_block->m_fileOwnerDataBlock = m_fileOwnerDataBlock;
        new_block->m_data_offset = start + m_data_offset;
    } else if(m_bMalloced) {
        new_block->m_data = (unsigned char *)m_data + start + m_data_offset;
    }
    new_block->m_bReadOnly = true;

    return new_block;
}

// Return a pointer to the start of the data block
void *KRDataBlock::getStart() {
    assertLocked();
    return m_data;
}

// Return a pointer to the byte after the end of the data block
void *KRDataBlock::getEnd() {
    assertLocked();
    return (unsigned char *)m_data + m_data_size;
}

// Return the size of the data block.  Use append() or expand() to make the data block larger
size_t KRDataBlock::getSize() const {
    return m_data_size;
}

// Expand the data block, and switch it to read-write mode.  Note - this may result in a mmap'ed file being copied to malloc'ed ram and then closed
void KRDataBlock::expand(size_t size)
{
    if(m_data == NULL && m_fdPackFile == 0) {
        // Starting with an empty data block; allocate memory on the heap
        m_data = malloc(size);
        assert(m_data != NULL);
        m_data_size = size;
        m_data_offset = 0;
        m_bMalloced = true;
    } else if(m_bMalloced) {
        // Starting with a malloc'ed data block; realloc it expand
        m_data = realloc(m_data, m_data_size + size);
        m_data_size += size;
    } else {
        // Starting with a mmap'ed data block, an encapsulated pointer, or a sub-block; copy it to ram before expanding to avoid updating the original file until save() is called
        // ... Or starting with a pointer reference, we must make our own copy and must not free the pointer
        void *pNewData = malloc(m_data_size + size);
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
void KRDataBlock::append(void *data, size_t size) {
    // Expand the data block
    expand(size);
    
    // Fill the new space with the data to append
    lock();
    memcpy((unsigned char *)m_data + m_data_size - size, data, size);
    unlock();
}


// Copy the entire data block to the destination pointer
void KRDataBlock::copy(void *dest) {
    copy(dest, 0, m_data_size);
}

// Copy a range of data to the destination pointer
void KRDataBlock::copy(void *dest, int start, int count) {
    if(m_lockCount == 0 && m_fdPackFile != 0) {
        // Optimization: If we haven't mmap'ed or malloced the data already, pread() it directly from the file into the buffer
        ssize_t r = pread(m_fdPackFile, dest, count, start + m_data_offset);
        assert(r != -1);
    } else {
        lock();
        memcpy((unsigned char *)dest, (unsigned char *)m_data + start, count);
        unlock();
    }
}

// Append data to the end of the block, increasing the size of the block and making it read-write.
void KRDataBlock::append(KRDataBlock &data) {
    data.lock();
    append(data.getStart(), data.getSize());
    data.unlock();
}

// Append string to the end of the block, increasing the size of the block and making it read-write.  The null terminating character is included
void KRDataBlock::append(const std::string &s)
{
    const char *szText = s.c_str();
    append((void *)szText, strlen(szText)+1);
}

// Save the data to a file.
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
            copy(pNewData);
            
            // Unmap the new file
            munmap(pNewData, m_data_size);
            
            // Close the new file
            close(fdNewFile);
        }
        return true;
    }
}

// Get contents as a string
std::string KRDataBlock::getString()
{
    KRDataBlock b;
    b.append(*this);
    b.append((void *)"\0", 1); // Ensure data is null terminated, to read as a string safely
    b.lock();
    std::string ret = std::string((char *)b.getStart());
    b.unlock();
    return ret;
}

// Lock the memory, forcing it to be loaded into a contiguous block of address space
void KRDataBlock::lock()
{
    if(m_lockCount == 0) {
        
        // Memory mapped file; ensure data is mapped to ram
        if(m_fdPackFile) {
            if(m_data_size < KRENGINE_MIN_MMAP) {
                m_data = malloc(m_data_size);
                assert(m_data != NULL);
                copy(m_data);
            } else {
                //fprintf(stderr, "KRDataBlock::lock - \"%s\" (%i)\n", m_fileOwnerDataBlock->m_fileName.c_str(), m_lockCount);
                
                // Round m_data_offset down to the next memory page, as required by mmap
                size_t alignment_offset = m_data_offset & (KRAKEN_MEM_PAGE_SIZE - 1);
                if ((m_mmapData = mmap(0, m_data_size + alignment_offset, m_bReadOnly ? PROT_READ : PROT_WRITE, MAP_SHARED, m_fdPackFile, m_data_offset - alignment_offset)) == (caddr_t) -1) {
                    int iError = errno;
                    switch(iError) {
                        case EACCES:
                            fprintf(stderr, "mmap failed with EACCES\n");
                            break;
                        case EBADF:
                            fprintf(stderr, "mmap failed with EBADF\n");
                            break;
                        case EMFILE:
                            fprintf(stderr, "mmap failed with EMFILE\n");
                            break;
                        case EINVAL:
                            fprintf(stderr, "mmap failed with EINVAL\n");
                            break;
                        case ENOMEM:
                            fprintf(stderr, "mmap failed with ENOMEM\n");
                            break;
                        case ENXIO:
                            fprintf(stderr, "mmap failed with ENXIO\n");
                            break;
                        case EOVERFLOW:
                            fprintf(stderr, "mmap failed with EOVERFLOW\n");
                            break;
                        default:
                            fprintf(stderr, "mmap failed with errno: %i\n", iError);
                            break;
                    }
                    assert(false); // mmap() failed.
                }
                m_mapCount++;
                m_mapSize += m_data_size;
                m_mapOverhead += alignment_offset + KRAKEN_MEM_ROUND_UP_PAGE(m_data_size + alignment_offset) - m_data_size + alignment_offset;
                fprintf(stderr, "Mapped: %i Size: %d Overhead: %d\n", m_mapCount, m_mapSize, m_mapOverhead);
                m_data = (unsigned char *)m_mmapData + alignment_offset;
            }
        }
    }
    m_lockCount++;
}

// Unlock the memory, releasing the address space for use by other allocations
void KRDataBlock::unlock()
{
    // We expect that the data block was previously locked
    assertLocked();
    
    
    if(m_lockCount == 1) {
        
        // Memory mapped file; ensure data is unmapped from ram
        if(m_fdPackFile) {
            if(m_data_size < KRENGINE_MIN_MMAP) {
                free(m_data);
                m_data = NULL;
            } else {
                //fprintf(stderr, "KRDataBlock::unlock - \"%s\" (%i)\n", m_fileOwnerDataBlock->m_fileName.c_str(), m_lockCount);
                
                munmap(m_mmapData, m_data_size);
                m_data = NULL;
                m_mmapData = NULL;
                m_mapCount--;
                m_mapSize -= m_data_size;
                size_t alignment_offset = m_data_offset & (KRAKEN_MEM_PAGE_SIZE - 1);
                m_mapOverhead -= alignment_offset + KRAKEN_MEM_ROUND_UP_PAGE(m_data_size + alignment_offset) - m_data_size + alignment_offset;
                fprintf(stderr, "Mapped: %i Size: %d Overhead: %d\n", m_mapCount, m_mapSize, m_mapOverhead);
            }
        }
    }
    m_lockCount--;
}

// Assert if not locked
void KRDataBlock::assertLocked()
{
    assert(m_lockCount > 0);
}
