#include "Audio.h"
#include "Window.h"

const int32_t Audio::CHANNEL_COUNT = (AUDIO_CHANNEL_COUNT == oboe::ChannelCount::Mono) ? 1 : 2;

oboe::DataCallbackResult
Audio::onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames)
{
    assert(AUDIO_FORMAT == oboe::AudioFormat::I16);
    int16_t* outputData = (int16_t*) audioData;
    int16_t sampleValue = 0;

    for(int32_t i = 0; i < numFrames; ++i)
    {
        for(int32_t j = 0; j < CHANNEL_COUNT; ++j)
        {
            for(auto it = sounds.begin(); it != sounds.end();)
            {
                assert((*it)->playIndex <= (*it)->nSamples);
                if((*it)->playIndex == (*it)->nSamples)
                {
                    (*it)->playIndex = 0;
                    assert((*it)->audioIndex != -1 && (*it)->audioIndex < sounds.size());
                    it = sounds.erasePop_back((*it)->audioIndex);
                    continue;
                }
                sampleValue += (*it)->getBuffer()[(*it)->playIndex++];
                ++it;
            }
            outputData[i] = sampleValue;
            sampleValue = 0;
        }
    }

    return oboe::DataCallbackResult::Continue;
}

Audio::Audio() : sounds{}
{
    audioBuilder.setDirection(oboe::Direction::Output);
    audioBuilder.setPerformanceMode(oboe::PerformanceMode::None);
    audioBuilder.setSharingMode(oboe::SharingMode::Exclusive);
    audioBuilder.setFormat(AUDIO_FORMAT);
    audioBuilder.setChannelCount(AUDIO_CHANNEL_COUNT);
    audioBuilder.setUsage(oboe::Usage::Game);
    audioBuilder.setSampleRate(Sound::SAMPLE_RATE);
    audioBuilder.setCallback(this);
}

bool Audio::startSnd()
{
    oboe::Result result = audioBuilder.openStream(&audioStream);
    if(result != oboe::Result::OK)
    {
        utils::logF("OBOE: Failed to create stream. Error: %s", oboe::convertToText(result));
        return false;
    }

    if(audioStream->getFormat() != AUDIO_FORMAT)
    {
        utils::log("OBOE: Failed to get the right audio format!");
        return false;
    }
    if(audioStream->getSampleRate() != Sound::SAMPLE_RATE)
    {
        utils::log("OBOE: Failed to get the right sample rate!");
        return false;
    }
    if(audioStream->getChannelCount() != AUDIO_CHANNEL_COUNT)
    {
        utils::log("OBOE: Failed to get the right channel count!");
        return false;
    }

    return true;
}

void Audio::stopSnd()
{
    if(audioStream != nullptr)
    {
        audioStream->close();
        audioStream = nullptr;
    }
}

void Audio::startStream()
{
    audioStream->requestStart();
}

void Audio::stopStream()
{
    audioStream->requestStop();
}

void Audio::enqueue(Sound* sound)
{
    assert(sounds.size() < sounds.capacity());
    sounds.push_back(sound);
    sound->audioIndex = sounds.size() - 1;
}

void Audio::dequeue(Sound* sound)
{
    assert(sound->audioIndex != -1 && sound->audioIndex < sounds.size());
    sounds.erasePop_back(sound->audioIndex);
}

void Audio::clear()
{
    sounds.clear();
}
