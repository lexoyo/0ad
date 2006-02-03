/*
	A basic_string derivative that works with uint16_t as its underlying char
	type.
*/
#ifndef utf16string_H
#define utf16string_H

// On Linux, wchar_t is 32-bit, so define a new version of it.
// We now use this code on Windows as well, because wchar_t is a
// native type and distinct from utf16_t.
#include <string>
#include "lib/types.h"

typedef uint16_t utf16_t;
typedef std::basic_string<utf16_t> utf16string;
typedef std::basic_stringstream<utf16_t> utf16stringstream;

namespace std {
	template<>
	struct char_traits<utf16_t>
	{
		typedef utf16_t		char_type;
		typedef int			int_type;
		typedef streampos	pos_type;
		typedef streamoff	off_type;
		typedef mbstate_t	state_type;

		static void assign(char_type& c1, const char_type& c2)
		{ c1 = c2; }

		static bool eq(const char_type& c1, const char_type& c2)
		{ return c1 == c2; }

		static bool lt(const char_type& c1, const char_type& c2)
		{ return c1 < c2; }

		static int compare(const char_type* s1, const char_type* s2, size_t n)
		{
			return memcmp(s1, s2, n*sizeof(char_type));
		}

		static size_t length(const char_type* s)
		{
			const char_type* end=s;
			while (*end) end++;
			return end-s;
		}

		static const char_type* find(const char_type* s, size_t n, const char_type& a)
		{
			size_t i;
			for (i=0;i<n;i++)
			{
				if (s[i] == a) return s+i;
			}
			return NULL;
		}

		static char_type* move(char_type* s1, const char_type* s2, size_t n)
		{
			return (char_type *)memmove(s1, s2, n*sizeof(char_type));
		}

		static char_type* copy(char_type* s1, const char_type* s2, size_t n)
		{
			return (char_type *)memcpy2(s1, s2, n*sizeof(char_type));
		}

		static char_type* assign(char_type* s, size_t n, char_type a)
		{
			while (n--)
			{
				s[n]=a;
			}
			return s;
		}

		static char_type to_char_type(const int_type& c)
		{ return (char_type)c; }

		static int_type to_int_type(const char_type& c)
		{ return (int_type)c; }

		static bool eq_int_type(const int_type& c1, const int_type& c2)
		{ return c1 == c2; }

		static int_type eof()
		{ return -1; }

		static int_type not_eof(const int_type& c)
		{ return (c == -1) ? 0 : c; }
	};
}

#endif
