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

	int fileSize = file.getSize();

	std::unique_ptr<char[]> fileContents = std::make_unique<char[]>(fileSize);
	file.read(fileContents.get(), fileSize);

	FileHeader* fileHeader = (FileHeader*)fileContents.get();

	assert(fileHeader->riffId == (uint)ChunkId::RIFF);
	assert(fileHeader->waveId == (uint)ChunkId::WAVE);

	short* sampleData = nullptr;
	uint sampleDataSize = 0;

	for (RiffIt it = RiffIt(fileHeader + 1, (fileHeader + 1) + fileHeader->size - 4); it && (!sampleData || !nChannels); ++it)
	{
		switch (it.getType())
		{
		case (uint)ChunkId::FMT:
		{
			Fmt* fmt = (Fmt*)it.getChunkData();
			nChannels = fmt->nChannles;

			assert(fmt->wFormatTag == 1); //NOTE: Only PCM music!
			assert(fmt->nSamplesPerSecond == 48000);
			assert(fmt->wBitsPerSample == 16);
			assert(fmt->nBlockAlign == (sizeof(short)*fmt->nChannles));
			break;
		}
		case (uint)ChunkId::DATA:
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
		samples.push_back(Vector<short>((size_t)nSamples));

		for (int i = 0; i < nSamples; ++i)
		{
			samples[0][i] = sampleData[i];
		}
	}
	else if (nChannels == 2)
	{
		samples.push_back(Vector<short>(nSamples / 2));
		samples.push_back(Vector<short>(nSamples / 2));

		for (int i = 0; i < nSamples;)
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

bool Sound::reloadFromFile(const String & filename)
{
	return loadFromFile(filename);
}

Sound::Sound()
{
}

const Vector<Vector<short>>& Sound::getSamples() const
{
	return samples;
}

const int Sound::getNSamples() const
{
	return nSamples;
}

long long Sound::getSize() const
{
	return (getNSamples() * sizeof(short));
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
	uint size = chunk->size;
	if (size % 2 != 0)
		++size;
	at += sizeof(Chunk) + size;
	return *this;
}

uint Sound::RiffIt::getChunkDataSize() const
{
	Chunk* chunk = (Chunk*)at;
	return chunk->size;
}

void * Sound::RiffIt::getChunkData() const
{
	return at + sizeof(Chunk);
}

uint Sound::RiffIt::getType() const
{
	Chunk* chunk = (Chunk*)at;
	return chunk->id;
}
