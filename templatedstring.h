#pragma once

#include "license.txt"
#include "mem.h"

#define __TOKENIZER_MINUS_SIGN		'-'
#define __TOKENIZER_DECIMAL_POINT	'.'

/** @return true if the given character is a digit */
#define isADigit(c)	(c >= '0' && c <= '9')

/** @return true if the given character is a hexadecimal digit */
#define ishexdigit(c)	(isADigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))

// TODO move print code to another object so that stdio.h can be removed
#include <stdio.h>

/**
 * Another String class
 * It does not inherit from {@link TemplateArray} because this structure may
 * have a reference to read-only memory, where a TemplatedArray is always
 * mutable.
 * @author mvaganov@hotmail.com
 */
template <typename TYPE>
class TemplatedString
{
private:
	/** ASCII data of the string (NOT null-terminated) */
	TYPE * data;
	/** number of ASCII elements in the string */
	int size;
	/** this bit is set if data was allocated using new and must be released during the destructor */
	static const int ALLOCATED = (1<<0);
	/** if the character array should be treated as read only */
	static const int IMMUTABLE = (1<<1);
	/**
	 * {@link #ALLOCATED}
	 */
	int flags;

	void setImmutable(){flags |= IMMUTABLE;}
public:
	/** @return if this string points at mutable memory */
	bool isMutable(){return (flags & IMMUTABLE) == 0;}
	/** case-insensitive strcmp */
	static int STRCMP(const TYPE * const & a, const TYPE * const & b){
		TYPE ca, cb;
#define TOLOWER(CHAR)	if(CHAR >= 'A' && CHAR <= 'Z')CHAR += ('a'-'A');
		for(int i = 0; a[i] || b[i]; ++i){
			ca = a[i];	cb = b[i];
			TOLOWER(ca)
			TOLOWER(cb)
			if(ca != cb){return ca-cb;}
		}
		return 0;
#undef TOLOWER
	}

	/**
	 * @param c a character to find in a string
	 * @param a_charField a string to find a character in
	 * @param a_numChars the length of the string
	 */
	static bool isIn(TYPE const & c, const TYPE * const & a_charField, int const & a_numChars){
		for(int i = 0; i < a_numChars; ++i)
		{
			if(c == a_charField[i])
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * @return true if the given string is a representation of an integer
	 * @param a_length how long the string is
	 */
	static bool isInteger(const TYPE * source, const int & length)
	{
		TYPE c;
		for(int i = ((source[0]!=__TOKENIZER_MINUS_SIGN)?0:1); i < length && source[i] != '\0'; ++i)
		{
			c = source[i];
			if(!isADigit(c))
			{
				return false;
			}
		}
		return true;
	}

	/**
	 * @return true if the given string is a representation of an integer
	 */
	static bool isInteger(const TYPE * source)
	{
		return isInteger(source, TemplatedString<TYPE>::length(source));
	}

	/**
	 * @return true if the given string is a representation of a hexadecimal value
	 * @param a_length how long the string is
	 */
	static bool isHex(const TYPE * source, const int & length)
	{
		if(length < 2 || source[0] != '0' || source[1] != 'x')
			return false;
		TYPE c;
		for(int i = 2; i < length && source[i] != '\0'; ++i)
		{
			c = source[i];
			if(!ishexdigit(c))
			{
				return false;
			}
		}
		return true;
	}

	/**
	 * @return true if the given string is a representation of an hexadecimal value
	 */
	static bool isHex(const TYPE * source)
	{
		return isHex(source, TemplatedString<TYPE>::length(source));
	}

	/**
	 * @return true if the given string is a representation of a float
	 * @param a_length how long the string is
	 */
	static bool isFloat(const TYPE * source, const int & length)
	{
		TYPE c;
		bool havReadDecimal = false;
		for(int i = ((source[0]!=__TOKENIZER_MINUS_SIGN)?0:1); i < length; ++i)
		{
			c = source[i];
			if(!isADigit(c)
			&& (havReadDecimal
			|| !(havReadDecimal = (c == __TOKENIZER_DECIMAL_POINT))))
			{
				return false;
			}
		}
		return true;
	}

	/** @return true if the given string is a representation of a float */
	static bool isFloat(const TYPE * a_source)
	{
		return isFloat(a_source, TemplatedString<TYPE>::length(a_source));
	}

	/**
	 * @param a_source read an integer variable out of the given string
	 * @param a_length how long the string is
	 */
	static int parseInt(const TYPE * source, const int & length)
	{
		int value = 0;
		TYPE c;
		int i = source[0] != __TOKENIZER_MINUS_SIGN ? 0 : 1;
		for(; i < length; ++i)
		{
			c = source[i];
			value *= 10;
			value += (c - '0');
		}
		if(source[0] == __TOKENIZER_MINUS_SIGN)
		{
			value *= -1;
		}
		return value;
	}

	/**
	 * @param a_source read an integer variable out of the given string
	 */
	static int parseInt(const TYPE * a_source)
	{
		return parseInt(a_source, TemplatedString<TYPE>::length(a_source));
	}

	/**
	 * @param a_source read a hex variable out of the given string
	 * @param a_length how long the string is
	 */
	static int parseHex(const TYPE * source, const int & length)
	{
		int value = 0;
		TYPE c;
		int i = (source[0] == '0' && source[1] == 'x')?2:0;
		for(; i < length; ++i)
		{
	//		for(int a = 0; a < i; ++a)putchar(source[a]);
	//		printf(" %c ",source[i]);
	//		for(int a = i+1; a < length; ++a)putchar(source[a]);
	//		printf("   %d, [%x]\n", value, value);
			c = source[i];
			if(c >= 'a' && c <= 'f')	c='0'+10+(c-'a');
			if(c >= 'A' && c <= 'F')	c='0'+10+(c-'A');
			value *= 16;
			value += (c - '0');
		}
	//	printf("[%x]\n", value);
		return value;
	}

	/**
	 * @param a_source read a hex variable out of the given string
	 */
	static int parseHex(const TYPE * a_source)
	{
		return parseHex(a_source, TemplatedString<TYPE>::length(a_source));
	}

	/**
	 * @param a_source read a floating point variable out of the given string
	 * @param a_length how long the string is
	 */
	static float parseFloat(const TYPE * a_source, int const & a_length)
	{
		float value = 0;
		int decimal = 0;
		TYPE c;
		int i = a_source[0] != __TOKENIZER_MINUS_SIGN ? 0 : 1;
		for(; i < a_length-1; ++i)
		{
			c = a_source[i];
			if(c != __TOKENIZER_DECIMAL_POINT)
			{
				value *= 10;
				if(decimal > 0)
				{
					decimal *= 10;
				}
				value += (c - '0');
			}else{
				decimal = 1;
			}
		}
		if(a_source[0] == __TOKENIZER_MINUS_SIGN)
		{
			value *= -1;
		}
		if(decimal > 0)
		{
			value /= decimal;
		}
		return value;
	}

	/**
	 * @param a_source read a floating point variable out of the given string
	 */
	static float parseFloat(const TYPE * a_source)
	{
		return parseFloat(a_source, TemplatedString<TYPE>::length(a_source));
	}

	static void copy(TYPE * const & a_dest, int const & a_length, const TYPE * const & a_source){
		for(int i = 0; i < a_length; ++i){
			a_dest[i] = a_source[i];
		}
	}

	/** makes a 'new' TYPE array that is a copy of the given TYPE array */
	static TYPE * copyCStr(const TYPE * const & a_string, int len)
	{
		TYPE * c = NEWMEM_ARR(TYPE, len+1);
		copy(c, len, a_string);
		c[len] = '\0';
		return c;
	}
private:
	/**
	 * @param a_str actual character array text to copy
	 * @param a_len how much of the given character array to use as this string
	 * @param a_makeCopy if true, will allocate separate memory for the string.
	 * if false, the string will be immutable.
	 */
	void init(const TYPE * const & a_str, int const & a_len, bool const & a_makeCopy)
	{
		flags = 0;
		size = a_len;
		data = !a_makeCopy?const_cast<TYPE*>(a_str):TemplatedString<TYPE>::copyCStr(a_str, a_len);
		if(a_makeCopy)
			flags |= ALLOCATED;
		else
			setImmutable();
	}
public:
	void print() const
	{
		TYPE * end = data+size;
		TYPE * c = data;
		while(c < end){putchar(*c++);}
	}
	void print(int a_size) const
	{
		//TYPE * end = data+size;
		int i = 0;
		while(i < size && i < a_size){
			if(data[i] == '\n')break;
			putchar(data[i++]);
		}
	}
	/** used for debug printing purposes */
	void printNoWhitespace() const
	{
		for(int i = 0; i < size; ++i){
			switch(data[i]){
			case '\0':	case '\a':	case '\b':
			case '\t':	case '\n':	case '\r':
			case ' ':	case -1:	break;
			default:	putchar(data[i]);	break;
			}
		}
	}
	/** @return a copy of the given c-string (null-terminated) */
	static TYPE * copyCStr(const TYPE * const & a_string)
	{
		int len = TemplatedString::length(a_string);
		return copyCStr(a_string, len);
	}
	/** @return length of the null terminated string */
	static int length(const TYPE * const & str)
	{
		int len = 0;
		while(str[len])++len;
		return len;
	}
	/** @param data BY REFERENCE, not a copy */
	TemplatedString(TYPE * data, int size)
		:data(data),size(size),flags(0)
	{setImmutable();}
	TemplatedString():data(0),size(0),flags(0)
	{}
	/** make this string a copy (it's own memory if a_makeCopy is true) of the given string segment */
	TemplatedString(TYPE * const & a_str, int const & a_len, bool const & a_makeCopy)
	{
		init(a_str, a_len, a_makeCopy);
	}
	/** make this string a copy (it's own memory if a_makeCopy is true) of the given null-terminated string */
	TemplatedString(TYPE * const & a_str, bool const & a_makeCopy)
	{
		init(a_str, TemplatedString::length(a_str), a_makeCopy);
	}
	/** make this string a reference to the given null-terminated string */
	TemplatedString(const TYPE * const & a_str)
	{
		init(a_str, TemplatedString<TYPE>::length(a_str), false);
	}
	/** make this string a reference to the given string. Immutable. */
	TemplatedString(TemplatedString const & str):flags(0)
	{
		init(str.data, str.size, false);
	}
	/** make this string a copy (it's own memory if a_makeCopy is true) to the given string */
	TemplatedString(TemplatedString & str, bool const & a_makeCopy):flags(0)
	{
		init(str.cstr(), str.length(), a_makeCopy);
	}
	/** @note dangerous! if string allocated memory, it will be released here */
	void setData(const TYPE * a_data, int length)
	{
		if(flags & ALLOCATED)
			this->~TemplatedString();
		init(a_data, length, false);
	}
	void setData(const TYPE * a_data)
	{
		if(flags & ALLOCATED)
			this->~TemplatedString();
		init(a_data, length(a_data), false);
	}
	/**
	 * @return the index of a_char in this string, starting the linear search
	 * from index a_start
	 */
	int indexOf(TYPE const & a_char, int const & a_start) const
	{
		int found = -1;
		for(int i = a_start; found < 0 && i < size; ++i)
		{
			if(data[i] == a_char)
				found = i;
		}
		return found;
	}
	/** @return the index of a_char in this string, starting the linear search from index a_start */
	inline int indexOf(TYPE const & a_char){
		return indexOf(a_char, 0);
	}
	~TemplatedString()
	{
		if((flags & ALLOCATED) != 0 && data){
			DELMEM_ARR(data);//delete [] data;
			data = 0;
			size = 0;
			flags &= ~ALLOCATED;
		}
	}

	/**
	 * @return the character at the given location
	 */
	inline TYPE & getByReference(int const & a_index){
		if(!isMutable()){
			printf("use getCONST\n");
			int i=0;i=1/i;}// use getCONST
		return data[a_index];
	}
	/** overloaded array index operator */
	inline TYPE & operator[](int const & a_index){
		if(!isMutable()){
			printf("[] use getCONST\n");
			int i=0;i=1/i;}// use getCONST
		return data[a_index];
	}

	/** @return value (by value) from the list at given index */
	inline TYPE getCONST(const int a_index) const {
		return data[a_index];
	}
	/** @param a_value what value to put in the given index */
	inline void set(int const & a_index, TYPE const & a_value){
		if(!isMutable()){printf("set wont work\n");int i=0;i=1/i;}	// do not allow modification of immutable strings
		data[a_index] = a_value;
	}
	/** @return the size of this string */
	inline int length() const {
		return size;
	}
	/** @return {@link length()} */
	inline int getLength() const {
		return length();
	}
	/** @return a string that is a sub-string (allocates no memory for the substring) */
	inline TemplatedString<TYPE> subString(int const & a_startIndex, int const & length){
		return TemplatedString<TYPE>(&(data[a_startIndex]), length);
	}
	/** @return a string that is a sub-string (allocates no memory for the substring) */
	inline TemplatedString<TYPE> subString(int const & a_startIndex){
		return subString(a_startIndex, size-a_startIndex);
	}
	/**
	 * @param a_length sets the string length to this value
	 * @note use with caution. increasing the size should not be allowed...
	 */
	void setLength(int const & a_length){
		size = a_length;
	}

	/** */
	const TYPE * cstr(){
		return data;
	}

	/** copy this string into the given mutable standard array */
	inline void writeIntoCharBuffer(TYPE * const & a_buffer, int const & bufferSize) const {
		int end = (size<(bufferSize-1))?size:(bufferSize-1);
		for(int i = 0; i < end; ++i){a_buffer[i]=data[i];}
		a_buffer[end]='\0';
	}

	/** @return the index where this String and the given c-string literal stop matching exactly */
	int sameUntil(const TYPE * const & b) const {
		int i;
		for(i=0;i<size && data[i]==b[i];++i);
		return i;
	}

	/** @return true if string begins with prefix */
	static bool isStartingWith(const TYPE * const & string, const TYPE * const & prefix, int const & prefixLength){
		int i;
		for(i=0;i<prefixLength && prefix[i]==string[i];++i);
		return i>=prefixLength;
	}

	/**
	 * @param a_token which token is being searched for
	 * @param a_possibleTokens null-terminated list of tokens
	 * @return -1 if the given token was not found in the list of possible tokens
	 */
	int isMulticharToken(const TYPE ** const & a_possibleTokens) const
	{
		int index;
		for(int i = 0; a_possibleTokens[i]; ++i)
		{
			index = this->sameUntil(a_possibleTokens[i]);
			if(a_possibleTokens[i][index] == '\0')
			//if(str->compare(possibleTokens[i]) == 0)
			//if(stringCompare(str, possibleTokens[i], strlen) == 0)
				return i;
		}
		return -1;
	}

	static int compare(const TYPE * a, const TYPE * b){
		while(*a && *a == *b){++a;++b;}
		return *a - *b;
	}

	static int compare(const TYPE * const & a, int const & size, const TYPE * const & b){
		for(int i = 0; (size > 0 && i < size) || b[i]; ++i)
		{
			if(a[i] != b[i])
				return a[i] - b[i];
		}
		return 0;
	}

	int compare(const TYPE * const & b) const {
		return compare(data, size, b);
	}

	void setToUpper()
	{
		if(!isMutable()){
			printf("cannot set immutable string to upper case\n");
			return;
		}
		for(int i = 0; i < this->size; ++i)
		{
			TYPE c = data[i];
			if(c >= 'a' && c <= 'z')
				data[i] = c-32;
		}
	}

	bool isInteger() const 	{	return isInteger(data, size);	}
	bool isFloat() const 	{	return isFloat(data, size);		}
	bool isHex() const 		{	return isHex(data, size);		}

	int parseInteger() const	{	return parseInt(data, size);	}
	float parseFloat() const	{	return parseFloat(data, size);	}
	int parseHex() const		{	return parseHex(data, size);	}
};

///**
//TODO find a place to put this code... or just remove it?
//<code>TYPE * FLAG_NAMES[] = {
//	"managed",
//	"int",
//	"float",
//	"str",
//	"in_literal",
//	"//",
//	"ignore_//",
//	"again",
//};
//	int flag = (1<<0)|(1<<2)|(1<<7);
//	// output: "(managed|float|again)"
//	printOnFlags(flag, FLAG_NAMES);</code><br>
// */
//void printOnFlags(int const & a_flag, char ** flagNames)
//{
//	int printed = 0;
//	printf("(");
//	for(int i = 0; i < 8; ++i)
//	{
//		if((a_flag & (1 << i)) != 0)
//		{
//			if(printed > 0)
//			{
//				printf("|");
//			}
//			printf("%s", flagNames[i]);
//			printed++;
//		}
//	}
//	printf(")");
//}

#undef __TOKENIZER_MINUS_SIGN
#undef __TOKENIZER_DECIMAL_POINT
#undef isADigit
#undef ishexdigit

typedef TemplatedString<char> String;
