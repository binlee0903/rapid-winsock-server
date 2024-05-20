#include "String.h"

size_t Length(const char* string);

String::String(const char* s)
{
	if (s == nullptr || s[0] == '\0')
	{
		mString = new char[1];
		mString[0] = '\0';
		mCapacity = 1;
		mLength = 0;
	}
	else
	{
		mLength = Length(s);
		mCapacity = mLength + 1;

		mString = new char[mCapacity];

		for (size_t i = 0; i < mLength + 1; i++)
		{
			mString[i] = s[i];
		}
	}
}

String::String(const String& other)
{
	mLength = other.mLength;
	mCapacity = other.mCapacity;

	mString = new char[mCapacity];

	for (size_t i = 0; i < mCapacity; i++)
	{
		mString[i] = other[i];
	}
}

String::~String()
{
	if (mString != nullptr)
	{
		delete[] mString;
	}
}

unsigned int String::GetLength() const
{
	return mLength;
}

unsigned int String::GetCapacity() const
{
	return mCapacity;
}

const char* String::GetCString()
{
	return mString;
}

void String::Append(const char* s)
{
	if (s == nullptr || s[0] == '\0')
	{
		return;
	}

	size_t tempLength = Length(s);

	char* temp = new char[mCapacity + tempLength];

	for (size_t i = 0; i < mLength; i++)
	{
		temp[i] = mString[i];
	}

	for (size_t i = 0; i < tempLength; i++)
	{
		temp[mLength + i] = s[i];
	}

	delete[] mString;

	mCapacity = mLength + tempLength + 1;
	mLength += tempLength;
	mString = temp;
	mString[mLength] = '\0';
}

int String::IndexOf(const char* s)
{
	if (s == nullptr || s[0] == '\0')
	{
		return -1;
	}

	size_t tempLength = Length(s);

	for (size_t i = 0; i < mLength; i++)
	{
		if (mString[i] == s[0])
		{
			for (size_t j = 0; j < tempLength; j++)
			{
				if (mString[i + j] != s[j])
				{
					break;
				}
			}

			return i;
		}
	}

	return -1;
}

int String::LastIndexOf(const char* s)
{
	if (s == nullptr || s[0] == '\0')
	{
		return -1;
	}

	size_t tempLength = Length(s);
	int lastIndex = -1;
	bool bFound = false;

	for (size_t i = 0; i < mLength; i++)
	{
		if (mString[i] == s[0])
		{
			for (size_t j = 0; j < tempLength; j++)
			{
				if (mString[i + j] != s[j])
				{
					bFound = false;
					break;
				}
				bFound = true;
			}

			if (bFound)
			{
				lastIndex = i;
			}
		}
	}

	return lastIndex;
}

void String::Interleave(const char* s)
{
	if (s == nullptr || s[0] == '\0')
	{
		return;
	}

	size_t tempLength = Length(s);

	mCapacity = mLength + tempLength + 1;
	char* tempChar = new char[mCapacity];

	size_t i = 0, j = 0;

	if (mLength > tempLength)
	{
		while (i < mLength)
		{
			if (mString[i] != '\0')
			{
				tempChar[j] = mString[i];
				j++;
			}

			if (i < tempLength && s[i] != '\0')
			{
				tempChar[j] = s[i];
				j++;
			}

			i++;
		}
	}
	else
	{
		while (i < tempLength)
		{
			if (i < mLength && mString[i] != '\0')
			{
				tempChar[j] = mString[i];
				j++;
			}

			if (s[i] != '\0')
			{
				tempChar[j] = s[i];
				j++;
			}

			i++;
		}
	}

	delete[] mString;
	mLength = j;
	mString = tempChar;
	mString[mCapacity - 1] = '\0';
}

char String::operator[](size_t i) const
{
	return mString[i];
}

bool String::RemoveAt(unsigned int index)
{
	if (index < 0 || index >= mLength)
	{
		return false;
	}

	if (mString == nullptr)
	{
		return false;
	}

	if (mLength == 0 && index == 0)
	{
		return false;
	}

	if (mLength == 1 && index == 0)
	{
		delete[] mString;

		mString = new char[1];
		mString[0] = '\0';
		mLength = 0;
		mCapacity = 1;

		return true;
	}

	char* tempChar = new char[mLength];
	int j = 0;

	for (size_t i = 0; i < mCapacity; i++)
	{
		if (i != index)
		{
			tempChar[j] = mString[i];
			j++;
		}
	}

	delete[] mString;

	mString = tempChar;
	mLength--;
	mCapacity--;

	return true;
}

void String::PadLeft(unsigned int totalLength)
{
	if (totalLength < 0 || totalLength < mLength)
	{
		return;
	}

	char* tempChar = new char[totalLength + 1];

	for (size_t i = 0; i < totalLength - mLength; i++)
	{
		tempChar[i] = ' ';
	}

	size_t i = 0, j = totalLength - mLength;

	while (j < totalLength)
	{
		tempChar[j] = mString[i];
		i++;
		j++;
	}
	tempChar[totalLength] = '\0';

	delete[] mString;
	mString = tempChar;
	mLength = totalLength;
	mCapacity = totalLength + 1;
}

void String::PadLeft(unsigned int totalLength, const char c)
{
	if (totalLength < 0 || totalLength < mLength)
	{
		return;
	}

	char* tempChar = new char[totalLength + 1];

	for (size_t i = 0; i < totalLength - mLength; i++)
	{
		tempChar[i] = c;
	}

	size_t i = 0, j = totalLength - mLength;

	while (j < totalLength)
	{
		tempChar[j] = mString[i];
		i++;
		j++;
	}
	tempChar[totalLength] = '\0';

	delete[] mString;
	mString = tempChar;
	mLength = totalLength;
	mCapacity = totalLength + 1;
}

void String::PadRight(unsigned int totalLength)
{
	if (totalLength < 0 || totalLength < mLength)
	{
		return;
	}

	char* tempChar = new char[totalLength + 1];

	for (size_t i = totalLength - 1; i >= mLength; i--)
	{
		tempChar[i] = ' ';
	}

	for (size_t i = 0; i < mLength; i++)
	{
		tempChar[i] = mString[i];
	}
	tempChar[totalLength] = '\0';

	delete[] mString;
	mString = tempChar;
	mLength = totalLength;
	mCapacity = totalLength + 1;
}

void String::PadRight(unsigned int totalLength, const char c)
{
	if (totalLength < 0 || totalLength < mLength)
	{
		return;
	}

	char* tempChar = new char[totalLength + 1];

	for (size_t i = totalLength - 1; i >= mLength; i--)
	{
		tempChar[i] = c;
	}

	for (size_t i = 0; i < mLength; i++)
	{
		tempChar[i] = mString[i];
	}
	tempChar[totalLength] = '\0';

	delete[] mString;
	mString = tempChar;
	mLength = totalLength;
	mCapacity = totalLength + 1;
}

void String::Reverse()
{
	if (mString == nullptr || mString[0] == '\0')
	{
		return;
	}

	char* left = &mString[0];
	char* right = &mString[mLength - 1];

	char temp = 0;

	while (left < right)
	{
		temp = *left;
		*left = *right;
		*right = temp;

		left++;
		right--;
	}
}

void String::operator=(const String& rhs)
{
	if (mString != nullptr)
	{
		delete[] mString;
	}

	mString = new char[rhs.mLength + 1];

	for (size_t i = 0; i < rhs.mLength; i++)
	{
		mString[i] = rhs.mString[i];
	}

	mLength = rhs.mLength;
	mCapacity = rhs.mCapacity;
}

bool String::operator==(const String& rhs) const
{
	if (mString == nullptr || rhs.mString == nullptr)
	{
		if (mString == rhs.mString)
		{
			return true;
		}

		return false;
	}
	else if (mString[0] == '\0' || rhs[0] == '\0')
	{
		if (mString[0] == rhs[0])
		{
			return true;
		}

		return false;
	}
	else if (mLength != rhs.mLength)
	{
		return false;
	}

	for (size_t i = 0; i < mLength; i++)
	{
		if (mString[i] != rhs[i])
		{
			return false;
		}
	}

	return true;
}

void String::ToLower()
{
	for (size_t i = 0; i < mLength; i++)
	{
		if (mString[i] >= 'A' && mString[i] <= 'Z')
		{
			mString[i] += 32;
		}
	}
}

void String::ToUpper()
{
	for (size_t i = 0; i < mLength; i++)
	{
		if (mString[i] >= 'a' && mString[i] <= 'z')
		{
			mString[i] -= 32;
		}
	}
}

size_t Length(const char* string)
{
	size_t length = 0;

	while (string[length] != '\0')
	{
		length++;
	}

	return length;
}