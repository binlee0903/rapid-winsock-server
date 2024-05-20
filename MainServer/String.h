#pragma once

class String final
{
public:
	String(const char* s);
	String(const String& other);
	~String();

	unsigned int GetLength() const;
	unsigned int GetCapacity() const;
	const char* GetCString();
	void Append(const char* s);
	int IndexOf(const char* s);
	int LastIndexOf(const char* s);
	void Interleave(const char* s);
	char operator[](size_t i) const;
	bool RemoveAt(unsigned int i);
	void PadLeft(unsigned int totalLength);
	void PadLeft(unsigned int totalLength, const char c);
	void PadRight(unsigned int totalLength);
	void PadRight(unsigned int totalLength, const char c);
	void Reverse();
	void operator=(const String& rhs);
	bool operator==(const String& rhs) const;
	void ToLower();
	void ToUpper();

private:
	char* mString;

	size_t mLength;
	size_t mCapacity;
};