#pragma once

#include "Utils.h"
#include "Vector.h"

#define CALL_STRING_ARRAY(exp) if(shortRep) return stringUnion.stringArrayShort.exp; else return stringUnion.stringArrayLong.exp
//NOTE: For now, there is no possibiliy for a wide-character string. I could potentialy shoot myself in the foot here for localization purporses

class String
{
public:
	static constexpr int SHORT_STRING_SIZE = 16;
	static constexpr size_t npos = -1;
//NOTE: Is protected for the copy/move-constructor of Short- / LongString!
protected:
	const bool shortRep;
private:
	union StringUnion
	{
		Array<char, SHORT_STRING_SIZE> stringArrayShort;
		Vector<char> stringArrayLong;

		~StringUnion();
	} stringUnion;
protected:
	String(size_t count, bool shortRepIn);
	String(size_t count, char c, bool shortRepIn);
	//String(const String& other, size_t pos, size_t count);
	//String(const char* s, size_t count);
public:
	String(size_t count);
	String(size_t count, char c);
	template <size_t N>
	String(const char(&s)[N], bool shortRepIn = (N <= SHORT_STRING_SIZE));
	String(const String& other);
	String(String&& other);
	String& operator=(const String& rhs);
	String& operator=(String&& rhs);
	template <size_t N>
	String& operator=(const char(&s)[N]);
	~String();

	char& at(size_t pos);
	char at(size_t pos) const;

	char& operator[](size_t pos);
	char operator[](size_t pos) const;
	
	char& front();
	char front() const;
	char& back();
	char back() const;
	const char* data() const;
	const char* c_str() const;

	Iterator<char> begin();
	Iterator<char> end();

	ConstIterator<char> begin() const;
	ConstIterator<char> end() const;

	bool empty() const;
	
	size_t size() const;
	size_t length() const;

	void reserve(size_t count);
	size_t capacity() const;

	void clear();
	
	//NOTE: Implement insert if you need them

	String& erase(size_t pos, size_t count);
	Iterator<char> erase(const Iterator<char>& pos);
	Iterator<char> erase(const Iterator<char>& first, const Iterator<char>& last);

	Iterator<char> push_back(char c);
	Iterator<char> pop_back();

	//NOTE: Will fail if string is "full" if it is a short string
	String& append(size_t count, char c);
	String& append(const String& str);
	String& append(const String& str, size_t pos, size_t count);
	String& append(const char* str);
	String& append(const char* str, size_t count);
	String& operator+=(const String& str);
	String& operator+=(char c);
	String& operator+=(const char* str);

	String substr(size_t pos = 0, size_t count = npos) const;

	size_t find(char c, size_t pos = 0) const;
	//TODO: IMPLEMENT!!!!! (Used in /*Physics::getAllCollisionIdsWhichContain (which is #ifed out now*/ and InkscapeAnimation)
	size_t find(const char* str, size_t pos = 0) const;
	size_t find(const String& str, size_t pos = 0) const;
	//NOTE: isn't it unnecessary?
	size_t find_first_of(char c, size_t pos = 0) const;
	size_t find_last_of(char c, size_t pos = npos) const;

	//NOTE: Implement resize if you need it!

	void swap(String& other);

	String operator+(const String& rhs) const;
	String operator+(const char* rhs) const;
	String operator+(char c) const;
	friend String operator+(const char* lhs, const String& rhs);
	friend String operator+(char lhs, const String& rhs);
	friend String operator+(String&& lhs, const String& rhs);

	bool operator==(const String& rhs) const;
	bool operator==(const char* rhs) const;
	friend bool operator==(const char* lhs, const String& rhs);
	bool operator!=(const String& rhs) const;
	bool operator!=(const char* rhs) const;
	friend bool operator!=(const char* lhs, const String& rhs);
};

template<size_t N>
inline String::String(const char(&s)[N], bool shortRepIn) : shortRep(shortRepIn), stringUnion{{ { {0} } }}
{
	if (shortRep)
	{
		assert(N <= SHORT_STRING_SIZE);
		for (size_t i = 0; i < N; ++i)
			stringUnion.stringArrayShort.push_back(s[i]);
	}
	else
	{
		new (&stringUnion.stringArrayLong) Vector<char>(N);

		for (size_t i = 0; i < N; ++i)
			stringUnion.stringArrayLong.push_back(s[i]);
	}
}

template<size_t N>
inline String & String::operator=(const char(&s)[N])
{
	this->~String();

	new (this) String(s);

	return *this;
}

struct ShortString : public String
{
	ShortString();
	ShortString(size_t count);
	ShortString(size_t count, char c);
	template <size_t N>
	ShortString(const char(&s)[N]);
	ShortString(const String& other);
	ShortString(String&& other);
	ShortString& operator=(const String& rhs);
	ShortString& operator=(String&& rhs);
};

struct LongString : public String
{
	LongString();
	LongString(size_t count);
	LongString(size_t count, char c);
	template <size_t N>
	LongString(const char(&s)[N]);
	LongString(const String& other);
	LongString(String&& other);
	LongString& operator=(const String& rhs);
	LongString& operator=(String&& rhs);
};

#include <string>

namespace std {
	template<>
	struct hash<String> {
	public:
		size_t operator()(const String &s) const
		{
			//TODO: Build own hasher!
			return std::hash<std::string>()(s.c_str());
		}
	};

	template<>
	struct hash<ShortString> {
	public:
		size_t operator()(const ShortString &s) const
		{
			//TODO: Build own hasher!
			return std::hash<std::string>()(s.c_str());
		}
	};

	template<>
	struct hash<LongString> {
	public:
		size_t operator()(const LongString &s) const
		{
			//TODO: Build own hasher!
			return std::hash<std::string>()(s.c_str());
		}
	};
}

namespace utils
{
	bool isWordInLine(const String & word, const String & lineContent);
	String getWordBetweenChars(const String& lineContent, char first, char last);
}

namespace StringUnitTest
{
	void runStringUnitTests();
	void runStdStringUnitTests();
}

template<size_t N>
inline ShortString::ShortString(const char(&s)[N]) : String(s, true)
{
}

template<size_t N>
inline LongString::LongString(const char(&s)[N]) : String(s, false)
{
}
