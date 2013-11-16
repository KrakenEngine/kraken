//
//  KRDataBlock.h
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

#ifndef KREngine_KRDataBlock_h
#define KREngine_KRDataBlock_h

#include "KREngine-common.h"

#define KRENGINE_MIN_MMAP 32768

class KRDataBlock {
public:
    KRDataBlock();
    KRDataBlock(void *data, size_t size);
    ~KRDataBlock();
    
    // Encapsulate a pointer.  Note - The pointer will not be free'ed
    bool load(void *data, size_t size);
    
    // Load a file into memory using mmap.  The data pointer will be protected as read-only until append() or expand() is called
    bool load(const std::string &path);
    
    // Save the data to a file.
    bool save(const std::string& path);
    
    // Create a KRDataBlock encapsulating a sub-region of this block.  The caller is responsible to free the object.
    KRDataBlock *getSubBlock(int start, int length);
    
    // Append data to the end of the block, increasing the size of the block and making it read-write.
    void append(void *data, size_t size);
    
    // Append data to the end of the block, increasing the size of the block and making it read-write.
    void append(KRDataBlock &data);
    
    // Append string to the end of the block, increasing the size of the block and making it read-write.  The null terminating character is included
    void append(const std::string &s);
    
    // Expand or shrink the data block, and switch it to read-write mode.  Note - this may result in a mmap'ed file being copied to malloc'ed ram and then closed
    void expand(size_t size);
    
    // Unload a file, releasing any mmap'ed file handles or malloc'ed ram that was in use
    void unload();
    
    // Return a pointer to the start of the data block
    void *getStart();
    
    // Return a pointer to the one byte after the end of the data block
    void *getEnd();
    
    // Return the size of the data block.  Use append() or expand() to make the data block larger
    size_t getSize() const;
    
    // Get the contents as a string
    std::string getString();
    
    // Copy the entire data block to the destination pointer
    void copy(void *dest);
    
    // Copy a range of data to the destination pointer
    void copy(void *dest, int start, int count);
    
    // Lock the memory, forcing it to be loaded into a contiguous block of address space
    void lock();
    
    // Unlock the memory, releasing the address space for use by other allocations
    void unlock();
    
private:
    void *m_data;
    size_t m_data_size;
    size_t m_data_offset;
    
    // For memory mapped objects:
    int m_fdPackFile;
    std::string m_fileName;
    KRDataBlock *m_fileOwnerDataBlock;
    void *m_mmapData;
    
    // For malloc'ed objects:
    bool m_bMalloced;
    
    // Lock refcount
    int m_lockCount;
    
    // Read-only allocation
    bool m_bReadOnly;
    
    // Assert if not locked
    void assertLocked();

};

#endif
