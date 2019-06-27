#pragma once

#include "Vector.h"
#include "String.h"

class Sound
{
#define RIFF_CODE(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

	enum class ChunkId
	{
		FMT = RIFF_CODE('f', 'm', 't', ' '),
		DATA = RIFF_CODE('d', 'a', 't', 'a'),
		RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
		WAVE = RIFF_CODE('W', 'A', 'V', 'E')
	};

#pragma pack(push, 1)
	struct FileHeader
	{
		uint32_t riffId;
		uint32_t size;
		uint32_t waveId;
	};

	struct Chunk
	{
		uint32_t id;
		uint32_t size;
	};

	struct Fmt
	{
		uint16_t wFormatTag;
		uint16_t nChannles;
		uint32_t nSamplesPerSecond;
		uint32_t nAvgBytesPerSec;
		uint16_t nBlockAlign;
		uint16_t wBitsPerSample;
		uint16_t cbSize;
		uint16_t wValidBitsPerSample;
		uint32_t dwChannelMask;
		uint16_t subFormat[16];
	};
#pragma pack(pop)

	class RiffIt
	{
		uint8_t* at;
		uint8_t* stop;
	public:
		RiffIt(void* at, void* stop);
		explicit operator bool() const;
		RiffIt& operator++();
		void* getChunkData() const;
		uint32_t getType() const;
		uint32_t getChunkDataSize() const;
	};
private:
	Vector<Vector<int16_t>> samples;
	int32_t nChannels = 0;
	int32_t nSamples = 0;
public:
	Sound(const String& filename);
	const Vector<Vector<int16_t>>& getSamples() const;
	const int32_t getNSamples() const;
	uint64_t getSize() const;
	const int16_t* getBuffer() const;
	explicit operator bool() const;
};