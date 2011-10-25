//
//  objpacker.h
//  objpack
//
//  Created by Kearwood Gilbert on 11-04-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#import <vector.h>
#import <stdint.h>

#ifndef OBJPACKER_H
#define OBJPACKER_H


class KROBJPacker {
public:
    KROBJPacker();
    ~KROBJPacker();
    void pack(const char *szPath);
    
private:
    typedef struct {
        char szTag[16];
        float minx, miny, minz, maxx, maxy, maxz;
        int32_t vertex_count;
        int32_t material_count;
    } pack_header;
    
    typedef struct {
        int32_t start_vertex;
        int32_t vertex_count;
        char szName[64];
    } pack_material;
    
    typedef struct {
        float x;
        float y;
        float z;
    } Vertex3D, Vector3D;
    
    typedef struct {
        float u;
        float v;
    } TexCoord;
    
    typedef struct {
        Vertex3D vertex;
        Vector3D normal;
        Vector3D tangent;
        TexCoord texcoord;
    } VertexData;
};

#endif // OBJPACKER_H