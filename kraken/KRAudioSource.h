//
//  KRAudioSource.h
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

#ifndef KRAUDIOSOURCE_H
#define KRAUDIOSOURCE_H

#include "KREngine-common.h"
#include "KRResource.h"
#include "KRNode.h"
#include "KRTexture.h"

class KRAudioSample;
class KRAudioBuffer;

class KRAudioSource : public KRNode {
public:
    static void InitNodeInfo(KrNodeInfo* nodeInfo);

    KRAudioSource(KRScene &scene, std::string name);
    virtual ~KRAudioSource();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    virtual void physicsUpdate(float deltaTime);
    
    void render(VkCommandBuffer& commandBuffer, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass);

    // ---- Audio Playback Controls ----
    
    // Start playback of audio at the current audio sample position.  If audio is already playing, this has no effect.
    // play() does not automatically seek to the beginning of the sample.  Call setAudioFrame( 0 ) first if you wish the playback to begin at the start of the audio sample.
    // If not set to looping, audio playback ends automatically at the end of the sample
    void play();
    
    // Stop playback of audio.  If audio is already stopped, this has no effect.
    // If play() is called afterwards, playback will continue at the current audio sample position.
    void stop();
    
    // Returns true if audio is playing.  Will return false if a non-looped playback has reached the end of the audio sample.
    bool isPlaying();
    
    // Returns the audio playback position in units of integer audio frames.
    __int64_t getAudioFrame();
    
    // Sets the audio playback position with units of integer audio frames.
    void setAudioFrame(__int64_t next_frame);
    
    // Gets the audio playback position with units of floating point seconds.
    float getAudioTime();
    
    // Sets the audio playback position with units of floating point seconds.
    void setAudioTime(float new_position);
    
    // Returns true if the playback will automatically loop
    bool getLooping();
    
    // Enable or disable looping playback; Audio source must be stopped and re-started for loop mode changes to take effect
    void setLooping(bool looping);
    
    // ---- End: Audio Playback Controls ----
    
    void setSample(const std::string &sound_name);
    std::string getSample();
    
    KRAudioSample *getAudioSample();
    
    float getGain();
    void setGain(float gain);
    
    float getPitch();
    void setPitch(float pitch);
    

    
    bool getIs3D();
    void setIs3D(bool is3D);
    
    // 3d only properties:
    float getReverb();
    void setReverb(float reverb);
    
    float getReferenceDistance();
    void setReferenceDistance(float reference_distance);
    
    float getRolloffFactor();
    void setRolloffFactor(float rolloff_factor);
    
    bool getEnableOcclusion();
    void setEnableOcclusion(bool enable_occlusion);
    
    bool getEnableObstruction();
    void setEnableObstruction(bool enable_obstruction);
    
    // ---- Siren Audio Engine Interface ----
    
    void advanceFrames(int frame_count);
    KRAudioBuffer *getBuffer();
    int getBufferFrame();
    
    void sample(int frame_count, int channel, float *buffer, float gain);
    
private:
    __int64_t m_start_audio_frame; // Global audio frame that matches the start of the audio sample playback; when paused or not playing, this contains a value of -1
    __int64_t m_paused_audio_frame; // When paused or not playing, this contains the local audio frame number.  When playing, this contains a value of -1
    int m_currentBufferFrame; // Siren Audio Engine frame number within current buffer
    void advanceBuffer();
    
    std::string m_audio_sample_name;
    
    KRAudioSample *m_audioFile;
    unsigned int m_sourceID;
    float m_gain;
    float m_pitch;
    bool m_looping;
    std::queue<KRAudioBuffer *> m_audioBuffers;
    int m_nextBufferIndex;
    bool m_playing;
    bool m_is3d;
    bool m_isPrimed;
    
    void prime();
    void queueBuffer();
    
    // 3d only properties:
    float m_referenceDistance;
    float m_reverb; // type ALfloat	0.0 (dry) - 1.0 (wet) (0-100% dry/wet mix, 0.0 default)
    float m_rolloffFactor;
    bool m_enable_occlusion;
    bool m_enable_obstruction;
};

#endif /* defined(KRAUDIOSOURCE_H) */
