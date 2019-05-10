#include "Ifstream.h"
#include "Utils.h"
#include "AssetManager.h"

AAssetManager* Ifstream::aassetManager = nullptr;

Ifstream::Ifstream(const String & filename)
{
	open(filename);
}

Ifstream::Ifstream()
{
}

Ifstream::Ifstream(Ifstream && other) : asset(std::exchange(other.asset, nullptr)),
										good(std::exchange(other.good, false))
{
}

Ifstream & Ifstream::operator=(Ifstream && rhs)
{
	close();

	asset = std::exchange(rhs.asset, nullptr);
	aassetManager = std::exchange(rhs.aassetManager, nullptr);
	good = std::exchange(rhs.good, false);

	return *this;
}

Ifstream::~Ifstream()
{
	close();
}

Ifstream::operator bool() const
{
	return good;
}

bool Ifstream::operator!() const
{
	return !(operator bool());
}

bool Ifstream::eof()
{
	return (AAsset_getRemainingLength64(asset) == 0);
}

void Ifstream::getline(String & line)
{
	line.clear();
	char c;
	while (!eof())
	{
		c = get();

		if (c == '\n')
			break;

		line += c;
	}
}

void Ifstream::readTempLine()
{
	for (char c = get(); (!eof()) && (c != '\n'); c = get());
}

long long Ifstream::getSize()
{
	return AAsset_getLength64(asset);
}

void Ifstream::open(const String & filename)
{
	asset = AAssetManager_open(aassetManager, filename.c_str(), AASSET_MODE_UNKNOWN);

	if (!asset)
	{
		good = false;
		return;
	}
}

const void* Ifstream::getFullData()
{
	const void* s = AAsset_getBuffer(asset);
	assert(s != nullptr);
	return s;
}

void Ifstream::close()
{
	if (asset != nullptr)
	{
		AAsset_close(asset);
		asset = nullptr;
	}
}

void Ifstream::read(void * s, uint n)
{
	int result = AAsset_read(asset, s, n);
	assert(result > 0);
}

char Ifstream::get()
{
	char c;
	read(&c, 1);
	return c;
}

std::streampos Ifstream::tellg()
{
	return std::streampos(getSize() - AAsset_getRemainingLength64(asset));
}

void Ifstream::seekg(uint pos)
{
	off64_t result = AAsset_seek64(asset, pos, SEEK_SET);
	assert(result != -1);
}

void Ifstream::seekg(uint off, SeekDir way)
{
	switch (way)
	{
		case SeekDir::beg:
		{
			off64_t result = AAsset_seek64(asset, off, SEEK_SET);
			assert(result != -1);
			break;
		}
		case SeekDir::cur:
		{
			off64_t result = AAsset_seek64(asset, off, SEEK_CUR);
			assert(result != -1);
			break;
		}
		case SeekDir::end:
		{
			off64_t result = AAsset_seek64(asset, off, SEEK_END);
			assert(result != -1);
			break;
		}
		default:
		{
			InvalidCodePath;
			break;
		}
	}
}

void Ifstream::setAassetManager(AAssetManager* aassetManagerIn)
{
	aassetManager = aassetManagerIn;
}
