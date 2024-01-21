//
//  Kraken Engine.h
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

// #include "KRTextureManager.h"
#include "KRMesh.h"
#include "KRScene.h"
#include "KRContext.h"
#include "KRCamera.h"

#include "KREngine-common.h"

typedef enum KREngineParameterType
{
  KRENGINE_PARAMETER_INT, KRENGINE_PARAMETER_FLOAT, KRENGINE_PARAMETER_BOOL
} KREngineParameterType;

namespace kraken {
void set_parameter(const std::string& parameter_name, float parameter_value);
void set_debug_text(const std::string& print_text);
};

#ifdef __OBJC__

@interface KREngine : NSObject

+ (KREngine*)sharedInstance;

@property(nonatomic, readonly) NSDictionary* parameter_names;
@property(nonatomic, assign) KRContext* context;
@property(nonatomic, retain) NSString* debug_text;
@property(nonatomic, assign, readonly) KRRenderSettings* settings;

-(id)init;
-(BOOL)loadResource:(NSString*)path;

// Parameter enumeration interface
-(int)getParameterCount;
-(NSString*)getParameterNameWithIndex: (int)i;
-(NSString*)getParameterLabelWithIndex: (int)i;
-(KREngineParameterType)getParameterTypeWithIndex: (int)i;
-(float)getParameterMinWithIndex: (int)i;
-(float)getParameterMaxWithIndex: (int)i;
-(float)getParameterValueWithIndex: (int)i;
-(void)setParameterValueWithIndex: (int)i Value : (float)v;
-(void)setParameterValueWithName: (NSString*)name Value : (float)v;
-(int)getParameterIndexWithName: (NSString*)name;

-(void)renderScene: (KRScene*)pScene WithDeltaTime : (float)deltaTime AndWidth : (int)width AndHeight : (int)height AndDefaultFBO : (GLint)defaultFBO;
//- (void)renderScene: (KRScene *)pScene WithDeltaTime: (float)deltaTime;
-(void)setNearZ: (float)dNearZ;
-(void)setFarZ: (float)dFarZ;

@end

#endif

