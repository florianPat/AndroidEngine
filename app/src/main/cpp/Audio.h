#pragma once

#include <oboe/Oboe.h>
#include "Sound.h"

class Audio : public oboe::AudioStreamCallback
{
    static const int32_t CHANNEL_COUNT;
    static constexpr oboe::AudioFormat AUDIO_FORMAT = oboe::AudioFormat::I16;
    static constexpr oboe::ChannelCount AUDIO_CHANNEL_COUNT = oboe::ChannelCount::Mono;

    oboe::AudioStreamBuilder audioBuilder;
    oboe::AudioStream* audioStream = nullptr;
    Array<Sound*, 32> sounds;
public:
    Audio();
    Audio(const Audio& other) = delete;
    Audio& operator=(const Audio& rhs) = delete;
    Audio(Audio&& other) = delete;
    Audio& operator=(Audio&& rhs) = delete;
    bool startSnd();
    void stopSnd();
    void startStream();
    void stopStream();
    //TODO: Think about doing this thread safe!
    void start(Sound* sound);
    void stop(Sound* sound);
    void clear();
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames) override;
};
