#include "String.h"

String::String(size_t count, bool shortRepIn) : shortRep(shortRepIn), stringUnion{ { { {0} } } }
{
	if (!shortRep)
	{
		new (&stringUnion.stringArrayLong) Vector<char>(count);
		stringUnion.stringArrayLong.push_back('\0');
	}
	else
	{
		//NOTE: I can just ignore count here
		//assert(count < SHORT_STRING_SIZE);
		stringUnion.stringArrayShort.push_back('\0');
	}
}

String::String(size_t count, char c, bool shortRepIn) : shortRep(shortRepIn), stringUnion{{ { {0} } }}
{
	if (!shortRep)
	{
		new (&stringUnion.stringArrayLong) Vector<char>(count, c);
		stringUnion.stringArrayLong.push_back('\0');
	}
	else
	{
		assert(count < SHORT_STRING_SIZE);
		stringUnion.stringArrayShort.insert(0, count, c);
		stringUnion.stringArrayShort.push_back('\0');
	}
}

String::String(size_t count) : String(count, (count < SHORT_STRING_SIZE))
{
}

String::String(size_t count, char c) : String(count, c, (count < SHORT_STRING_SIZE))
{
}

String::String(const String & other) : shortRep(other.shortRep), stringUnion{{ { {0} } }}
{
	if (shortRep)
		new (&stringUnion.stringArrayShort) Array<char, SHORT_STRING_SIZE>(other.stringUnion.stringArrayShort);
	else
		new (&stringUnion.stringArrayLong) Vector<char>(other.stringUnion.stringArrayLong);
}

String::String(String && other) : shortRep(other.shortRep), stringUnion{{ { {0} } }}
{
	if (shortRep)
		new (&stringUnion.stringArrayShort) Array<char, SHORT_STRING_SIZE>(std::move(other.stringUnion.stringArrayShort));
	else
		new (&stringUnion.stringArrayLong) Vector<char>(std::move(other.stringUnion.stringArrayLong));
}

String & String::operator=(const String & rhs)
{
	assert(rhs.shortRep == shortRep);

	this->~String();

	new (this) String(rhs);

	return *this;
}

String & String::operator=(String && rhs)
{
	this->~String();

	new (this) String(std::move(rhs));

	return *this;
}

String::~String()
{
	if (!shortRep)
		stringUnion.stringArrayLong.~Vector();
}

char & String::at(size_t pos)
{
	assert(pos < size());

	CALL_STRING_ARRAY(at(pos));
}

char String::at(size_t pos) const
{
	CALL_STRING_ARRAY(at(pos));
}

char & String::operator[](size_t pos)
{
	CALL_STRING_ARRAY(operator[](pos));
}

char String::operator[](size_t pos) const
{
	CALL_STRING_ARRAY(operator[](pos));
}

char & String::front()
{
	CALL_STRING_ARRAY(front());
}

char String::front() const
{
	CALL_STRING_ARRAY(front());
}

char & String::back()
{
	CALL_STRING_ARRAY(at(size() - 1));
}

char String::back() const
{
	CALL_STRING_ARRAY(at(size() - 1));
}

const char * String::data() const
{
	CALL_STRING_ARRAY(data());
}

const char * String::c_str() const
{
	CALL_STRING_ARRAY(data());
}

Iterator<char> String::begin()
{
	CALL_STRING_ARRAY(begin());
}

Iterator<char> String::end()
{
	CALL_STRING_ARRAY(end() - 1);
}

ConstIterator<char> String::begin() const
{
	CALL_STRING_ARRAY(begin());
}

ConstIterator<char> String::end() const
{
	CALL_STRING_ARRAY(end() - 1);
}

bool String::empty() const
{
	return (size() == 0);
}

size_t String::size() const
{
	CALL_STRING_ARRAY(size() - 1);
}

size_t String::length() const
{
	CALL_STRING_ARRAY(size() - 1);
}

void String::reserve(size_t count)
{
	assert(!shortRep);

	stringUnion.stringArrayLong.reserve(count);
}

size_t String::capacity() const
{
	CALL_STRING_ARRAY(capacity());
}

String::StringUnion::~StringUnion()
{
	//NOTE: Does nothing but needs to exist to not implicitly delete it
}

void String::clear()
{
	if (shortRep)
	{
		stringUnion.stringArrayShort.clear();
		stringUnion.stringArrayShort.push_back('\0');
	}
	else
	{
		stringUnion.stringArrayLong.clear();
		stringUnion.stringArrayLong.push_back('\0');
	}
}

String & String::erase(size_t pos, size_t count)
{
	assert((pos + count) <= size());

	if (shortRep)
		stringUnion.stringArrayShort.erase(pos, pos + count);
	else
		stringUnion.stringArrayLong.erase(pos, pos + count);

	return *this;
}

Iterator<char> String::erase(const Iterator<char>& pos)
{
	assert(pos.getIndex() <= size());

	CALL_STRING_ARRAY(erase(pos));
}

Iterator<char> String::erase(const Iterator<char>& first, const Iterator<char>& last)
{
	assert(last.getIndex() <= size());

	CALL_STRING_ARRAY(erase(first, last));
}

Iterator<char> String::push_back(char c)
{
	CALL_STRING_ARRAY(insertPush_back(size(), c));
}

Iterator<char> String::pop_back()
{
	CALL_STRING_ARRAY(erasePop_back(size()));
}

String & String::append(size_t count, char c)
{
	if (shortRep)
		stringUnion.stringArrayShort.insert(size(), count, c);
	else
		stringUnion.stringArrayLong.insert(size(), count, c);

	return *this;
}

String & String::append(const String & str)
{
	if (shortRep)
	{
		stringUnion.stringArrayShort.pop_back();

		for (char c : str)
		{
			stringUnion.stringArrayShort.push_back(c);
		}

		stringUnion.stringArrayShort.push_back('\0');
	}
	else
	{
		stringUnion.stringArrayLong.pop_back();

		for (char c : str)
		{
			stringUnion.stringArrayLong.push_back(c);
		}

		stringUnion.stringArrayLong.push_back('\0');
	}

	return *this;
}

String & String::append(const String & str, size_t pos, size_t count)
{
	assert((pos + count) < str.size());

	if (shortRep)
	{
		stringUnion.stringArrayShort.pop_back();

		size_t i = 0;
		for (auto it = (str.begin() + pos); i < count; ++i, ++it)
		{
			stringUnion.stringArrayShort.push_back(*it);
		}

		stringUnion.stringArrayShort.push_back('\0');
	}
	else
	{
		stringUnion.stringArrayLong.pop_back();

		size_t i = 0;
		for (auto it = (str.begin() + pos); i < count; ++i, ++it)
		{
			stringUnion.stringArrayLong.push_back(*it);
		}

		stringUnion.stringArrayLong.push_back('\0');
	}

	return *this;
}

String & String::append(const char * str)
{
	if (shortRep)
	{
		stringUnion.stringArrayShort.pop_back();

		for (size_t i = 0; str[i] != '\0'; ++i)
		{
			stringUnion.stringArrayShort.push_back(str[i]);
		}

		stringUnion.stringArrayShort.push_back('\0');
	}
	else
	{
		stringUnion.stringArrayLong.pop_back();

		for (size_t i = 0; str[i] != '\0'; ++i)
		{
			stringUnion.stringArrayLong.push_back(str[i]);
		}

		stringUnion.stringArrayLong.push_back('\0');
	}

	return *this;
}

String & String::append(const char * str, size_t count)
{
	if (shortRep)
	{
		stringUnion.stringArrayShort.pop_back();

		for (size_t i = 0; str[i] != '\0' && i < count; ++i)
		{
			stringUnion.stringArrayShort.push_back(str[i]);
		}

		stringUnion.stringArrayShort.push_back('\0');
	}
	else
	{
		stringUnion.stringArrayLong.pop_back();

		for (size_t i = 0; str[i] != '\0' && i < count; ++i)
		{
			stringUnion.stringArrayLong.push_back(str[i]);
		}

		stringUnion.stringArrayLong.push_back('\0');
	}

	return *this;
}

String & String::operator+=(const String & str)
{
	return append(str);
}

String & String::operator+=(char c)
{
	return append(1, c);
}

String & String::operator+=(const char * str)
{
	return append(str);
}

String String::substr(size_t pos, size_t count) const
{
	if (count == npos)
		count = size() - pos;

	assert((pos + count) <= size());

	String result(count + 1);

	for (size_t i = pos; i < (pos + count); ++i)
	{
		result.push_back(this->at(i));
	}

	return result;
}

size_t String::find(char c, size_t pos) const
{
	for (size_t i = pos; i < size(); ++i)
	{
		if (at(i) == c)
			return i;
	}

	return npos;
}

size_t String::find(const String & str, size_t pos) const
{
    return find(str.c_str(), pos, str.size());
}

size_t String::find_first_of(char c, size_t pos) const
{
	return find(c, pos);
}

size_t String::find_last_of(char c, size_t pos) const
{
	assert(size() > 0);

	if (pos == npos)
		pos = size() - 1;

	for (int i = pos; i >= 0; --i)
	{
		if (at(i) == c)
			return i;
	}

	return npos;
}

void String::swap(String & other)
{
	String temp = std::move(other);
	other = std::move(*this);
	*this = std::move(temp);
}

String String::operator+(const String & rhs) const
{
	String lhs(*this);

	lhs += rhs;

	return lhs;
}

String String::operator+(const char * rhs) const
{
	String lhs(*this);

	lhs += rhs;

	return lhs;
}

String String::operator+(char c) const
{
	String lhs(*this);

	lhs += c;

	return lhs;
}

bool String::operator==(const String & rhs) const
{
	return operator==(rhs.c_str());
}

bool String::operator==(const char * rhs) const
{
	for (size_t i = 0; (i < size()) && (rhs[i] != '\0'); ++i)
	{
		if (at(i) != rhs[i])
			return false;
	}

	return true;
}

bool String::operator!=(const String & rhs) const
{
	return !(this->operator==(rhs));
}

bool String::operator!=(const char * rhs) const
{
	return !(this->operator==(rhs));
}

String operator+(const char * lhs, const String & rhsIn)
{
	String rhs(rhsIn);
	rhs += lhs;

	return rhs;
}

String operator+(char lhs, const String & rhsIn)
{
	String rhs(rhsIn);
	rhs += lhs;

	return rhs;
}

String operator+(String && lhs, const String & rhs)
{
	return lhs += rhs;
}

bool operator==(const char * lhs, const String & rhs)
{
	return (rhs == lhs);
}

bool operator!=(const char * lhs, const String & rhs)
{
	return !(rhs == lhs);
}

size_t String::find(const char* str, size_t pos, size_t strSize) const
{
    if(str[0] == '\0')
        return npos;

    const size_t thisSize = size();
    if(pos >= thisSize)
    	return npos;

    size_t result = npos;
    for(size_t i = pos; (result == npos) && ((thisSize - i) >= strSize) && (i < size()); ++i)
    {
        result = i;
        for(size_t j = 0, k = i; str[j] != '\0'; ++j, ++k)
        {
            if(at(k) == str[j])
                continue;
            else
            {
                result = npos;
                break;
            }
        }
    }

    return result;
}

String::String(const String& other, bool shortRepIn) : shortRep(shortRepIn), stringUnion{{ { {0} } }}
{
	if(shortRep)
	{
		assert(other.size() < SHORT_STRING_SIZE);
		if(other.shortRep)
			new (&stringUnion.stringArrayShort) Array<char, SHORT_STRING_SIZE>(other.stringUnion.stringArrayShort);
		else
		{
			for (size_t i = 0; i < other.size(); ++i)
				stringUnion.stringArrayShort.push_back(other[i]);

			stringUnion.stringArrayShort.push_back('\0');
		}
	}
	else
	{
		if(other.shortRep)
		{
			new (&stringUnion.stringArrayLong) Vector<char>();

			for (size_t i = 0; i < other.size(); ++i)
				stringUnion.stringArrayLong.push_back(other[i]);

			stringUnion.stringArrayLong.push_back('\0');
		}
		else
			new (&stringUnion.stringArrayLong) Vector<char>(other.stringUnion.stringArrayLong);
	}
}

bool utils::isWordInLine(const String & word, const String & lineContent)
{
	size_t o = 0;
	bool result = false;
	while (o < lineContent.size() && !result)
	{
		o = lineContent.find(word[0], o);
		String searchWord(word);
		auto it = ++searchWord.begin();
		for (; o < lineContent.size(); ++it)
		{
			if (it != searchWord.end())
			{
				if (lineContent.at(++o) == it[0])
					continue;
				else
					break;
			}
			else
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

String utils::getWordBetweenChars(const String& lineContent, char firstChar, char lastChar)
{
	size_t first = lineContent.find_first_of(firstChar);
	assert(first != String::npos);
	++first;
	size_t last = lineContent.find_first_of(lastChar, first);
	assert(last != String::npos);
	String result(last - first);
	result = lineContent.substr(first, last - first);
	return result;
}

LongString utils::getDirPathToFile(const String& str)
{
	if(size_t lastSlashPos = str.find_last_of('/'); lastSlashPos != String::npos)
		return LongString(str.substr(0, lastSlashPos + 1));
	else
		return "";
}

#include "UnitTester.h"

void StringUnitTest::runStringUnitTests()
{
#if 0
	UnitTester::test([]() {
		String string(15);
		String string2 = string;
		unitAssert(string2.size() == 0);
		unitAssert(string.capacity() == 16);
		unitAssert(string.empty() == true);
		
		String str("Hello World");
		str.pop_back();
		unitAssert(str.back() == 'l');
		str.erase(0, 3);
		unitAssert((*str.begin()) == 'l');
		unitAssert(str.length() == 7);
		unitAssert(str.substr(0, 3) == "lo ");
		str.append("nicenice");
		unitAssert(str.back() == 'e');
		unitAssert(str.size() == 15);
		unitAssert(str.find('n') == 7);
		unitAssert(str.find_last_of('n') == 11);

		String otherString("That is a really long string that does not fit on the stack");
		otherString.append("Yeah, relly cool");
		otherString.erase(0, 10);
		unitAssert(otherString.back() == 'l');
		unitAssert(otherString.front() == 'r');
		unitAssert(str != otherString);
		str = std::move(otherString);
		unitAssert(str.capacity() > 16);

		String niceString("hada");
		String epicString = str + niceString;
		
		char sum = 0;
		for (char c : str)
		{
			sum += c;
		}

		String string1("Hello!");
		String string3("Hello!");

		unitAssert(string1.find(string2, 0) == String::npos);
		unitAssert(string1.find(string3, 2) == String::npos);
		unitAssert(string1.find(string3, 0) == 0);
		unitAssert(string1.find("llo", 0) == 2);

		return true;
	});
#endif
}

void StringUnitTest::runStdStringUnitTests()
{
#if 0
#include <string>

	UnitTester::test([]() {
		std::string string;
		std::string string2 = string;
		unitAssert(string2.size() == 0);
		//unitAssert(string.capacity() == 15);
		unitAssert(string.empty() == true);

		std::string str("Hello World");
		str.pop_back();
		unitAssert(str.back() == 'l');
		str.erase(0, 3);
		unitAssert((*str.begin()) == 'l');
		unitAssert(str.length() == 7);
		unitAssert(str.substr(0, 3) == "lo ");
		str.append("nicenice");
		unitAssert(str.back() == 'e');
		unitAssert(str.size() == 15);
		unitAssert(str.find('n') == 7);
		unitAssert(str.find_last_of('n') == 11);

		std::string otherString("That is a really long string that does not fit on the stack");
		otherString.append("Yeah, relly cool");
		otherString.erase(0, 10);
		unitAssert(otherString.back() == 'l');
		unitAssert(otherString.front() == 'r');
		unitAssert(str != otherString);
		str = std::move(otherString);
		unitAssert(str.capacity() > 16);

		std::string niceString("hada");
		std::string epicString = str + niceString;

		char sum = 0;
		for (char c : str)
		{
			sum += c;
		}

        std::string string1("Hello!");
        std::string string3("Hello!");

        //NOTE: This results in 0, which I do not understand!
        unitAssert(string1.find(string2, 0) == 0);
        unitAssert(string1.find(string3, 2) == String::npos);
        unitAssert(string1.find(string3, 0) == 0);
        unitAssert(string1.find("llo", 0) == 2);

		return true;
	});
#endif
}

ShortString::ShortString() : String(0, true)
{
}

ShortString::ShortString(size_t count) : String(count, true)
{
}

ShortString::ShortString(size_t count, char c) : String(count, c, true)
{
}

ShortString::ShortString(const String & other) : String(other, true)
{
	assert(shortRep == true);
}

ShortString::ShortString(String && other) : String(other, true)
{
	assert(shortRep == true);
}

/*
ShortString & ShortString::operator=(const String & rhs)
{
	((String*)(this))->operator=(rhs);

	return *this;
}

ShortString & ShortString::operator=(String && rhs)
{
	((String*)(this))->operator=(std::move(rhs));

	return *this;
}
 */

LongString::LongString() : String(SHORT_STRING_SIZE, false)
{
}

LongString::LongString(size_t count) : String(count, false)
{
}

LongString::LongString(size_t count, char c) : String(count, c, false)
{
}

LongString::LongString(const String & other) : String(other, false)
{
	assert(shortRep == false);
}

LongString::LongString(String && other) : String(other, false)
{
	assert(shortRep == false);
}
