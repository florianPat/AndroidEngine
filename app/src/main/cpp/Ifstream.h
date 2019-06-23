#pragma once

#include "android_native_app_glue.h"
#include "String.h"

class Ifstream final
{
public:
	enum class SeekDir {beg, cur, end};
private:
	AAsset* asset = nullptr;
	static AAssetManager* aassetManager;
	bool good = true;
public:
	Ifstream(const String& filename);
	Ifstream();
	Ifstream(const Ifstream& other) = delete;
	Ifstream(Ifstream&& other);
	Ifstream& operator=(const Ifstream& rhs) = delete;
	Ifstream& operator=(Ifstream&& rhs);
	~Ifstream();
	explicit operator bool() const;
	bool operator!() const;
	bool eof();
	void getline(String& line);
	void readTempLine();
	void read(void* s, uint32_t n);
	char get();
	std::streampos tellg();
	void seekg(uint32_t pos);
	void seekg(uint32_t off, SeekDir way);
	uint64_t getSize();
	void open(const String& filename);
	const void* getFullData();
	void close();
	static void setAassetManager(AAssetManager* aassetManagerIn);
};