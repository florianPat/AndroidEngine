#include "Sound.h"
#include "Ifstream.h"
#include "Utils.h"
#include <memory>

bool Sound::loadFromFile(const String & filename)
{
	Ifstream file;
	file.open(filename);

	if (!file)
	{
		utils::logBreak("Could not open file!");
	}

	//TODO:!!!!
	uint32_t fileSize = (uint32_t)file.getSize();

	std::unique_ptr<int8_t[]> fileContents = std::make_unique<int8_t[]>(fileSize);
	file.read(fileContents.get(), fileSize);

	FileHeader* fileHeader = (FileHeader*)fileContents.get();

	assert(fileHeader->riffId == (uint)ChunkId::RIFF);
	assert(fileHeader->waveId == (uint)ChunkId::WAVE);

	short* sampleData = nullptr;
	uint32_t sampleDataSize = 0;

	for (RiffIt it = RiffIt(fileHeader + 1, (fileHeader + 1) + fileHeader->size - 4); it && (!sampleData || !nChannels); ++it)
	{
		switch (it.getType())
		{
			case (uint32_t)ChunkId::FMT:
			{
				Fmt* fmt = (Fmt*)it.getChunkData();
				nChannels = fmt->nChannles;

				assert(fmt->wFormatTag == 1); //NOTE: Only PCM music!
				assert(fmt->nSamplesPerSecond == 48000);
				assert(fmt->wBitsPerSample == 16);
				assert(fmt->nBlockAlign == (sizeof(short)*fmt->nChannles));
				break;
			}
			case (uint32_t)ChunkId::DATA:
			{
				sampleData = (short*)it.getChunkData();
				sampleDataSize = it.getChunkDataSize();
				break;
			}
		}
	}

	assert(nChannels && sampleData && sampleDataSize);

	nSamples = sampleDataSize / (nChannels * sizeof(short));

	nChannels = 1;

	if (nChannels == 1)
	{
		samples.push_back(Vector<short>((uint32_t)nSamples));

		for (int32_t i = 0; i < nSamples; ++i)
		{
			samples[0][i] = sampleData[i];
		}
	}
	else if (nChannels == 2)
	{
		samples.push_back(Vector<short>(nSamples / 2));
		samples.push_back(Vector<short>(nSamples / 2));

		for (int32_t i = 0; i < nSamples;)
		{
			samples[0][i] = sampleData[i];
			++i;
			samples[1][i] = sampleData[i];
			++i;
		}
	}
	else
		utils::logBreak("Invalid channel count!");

	return true;
}

const Vector<Vector<short>>& Sound::getSamples() const
{
	return samples;
}

const int32_t Sound::getNSamples() const
{
	return nSamples;
}

uint64_t Sound::getSize() const
{
	return (getNSamples() * sizeof(short) + sizeof(Sound));
}

const short * Sound::getBuffer() const
{
	return samples.data()->data();
}

Sound::operator bool() const
{
	return (nSamples != 0);
}

Sound::RiffIt::RiffIt(void * at, void* stop) : at(reinterpret_cast<uint8_t*>(at)), stop(reinterpret_cast<uint8_t*>(stop))
{
}

Sound::RiffIt::operator bool() const
{
	return this->at < this->stop;
}

Sound::RiffIt& Sound::RiffIt::operator++()
{
	Chunk* chunk = (Chunk*)at;
	uint32_t size = chunk->size;
	if (size % 2 != 0)
		++size;
	at += sizeof(Chunk) + size;
	return *this;
}

uint32_t Sound::RiffIt::getChunkDataSize() const
{
	Chunk* chunk = (Chunk*)at;
	return chunk->size;
}

void * Sound::RiffIt::getChunkData() const
{
	return at + sizeof(Chunk);
}

uint32_t Sound::RiffIt::getType() const
{
	Chunk* chunk = (Chunk*)at;
	return chunk->id;
}
