//=============================================================================
//  FILE:           unicode.h
//  SYSTEM:         
//  DESCRIPTION:
//-----------------------------------------------------------------------------
//  COPYRIGHT:      (C)Copyright 2024 Adrian Purser. All Rights Reserved.
//  LICENCE:        MIT
//  MAINTAINER:     AJP - Adrian Purser <ade@arcadestuff.com>
//  CREATED:        26-JUN-2024 Adrian Purser <ade@arcadestuff.com>
//=============================================================================
#ifndef GUARD_ADE_XML_UNICODE_H
#define GUARD_ADE_XML_UNICODE_H

#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>
#include <optional>

namespace adexml::unicode
{

static constexpr uint32_t UNI_RELACEMENT_CHAR		=	0x00000FFFDUL;
static constexpr uint32_t UNI_MAX_BMP						=	0x00000FFFFUL;
static constexpr uint32_t UNI_MAX_UTF16					=	0x00010FFFFUL;
static constexpr uint32_t UNI_MAX_UTF32					=	0x07FFFFFFFUL;
static constexpr uint32_t UNI_MAX_LEGAL_UTF32		=	0x00010FFFFUL;
static constexpr uint32_t UNI_SUR_HIGH_START		=	0x00000D800UL;
static constexpr uint32_t UNI_SUR_HIGH_END			=	0x00000DBFFUL;
static constexpr uint32_t UNI_SUR_LOW_START			=	0x00000DC00UL;
static constexpr uint32_t UNI_SUR_LOW_END				=	0x00000DFFFUL;


inline size_t
code_length(char8_t u8)
{
	static const size_t trailingbytes[] = { 1,1,1,1,1,1,1,1,2,2,2,2,3,3,4,5 };
	return u8 & 0x80 ? trailingbytes[(u8>>2)&0x0F]+1 : 1;
}

class U8Parser
{
private:
	size_t						m_codelen = 0;
	char32_t					m_code 		= 0;

public:
	std::optional<char32_t> put(char8_t u8)
		{
			static const size_t trailingbytes[] = { 1,1,1,1,1,1,1,1,2,2,2,2,3,3,4,5 };

			if(m_codelen == 0)
			{
				if((u8 & 0x80) == 0)
					return static_cast<char32_t>(u8);
				m_codelen = trailingbytes[(u8>>2)&0x0F];
				m_code 		= u8 & (0x1F >> (m_codelen-1));
				return {};
			}

			// If the next code isn't 10xxxxxx then reset since it's an error.
			if((u8 & 0xC0) != 0x80)
			{
				m_codelen = 0;
				return {};
			}

			m_code = (m_code << 6) | (u8 & 0x3F);
			--m_codelen;

			if(m_codelen == 0)
				return m_code;

			return {};
		};
};


inline
int
u8_to_u32( const std::u8string & u8, std::u32string & out_u32)
{
	static const int	trailingbytes[] = { 1,1,1,1,1,1,1,1,2,2,2,2,3,3,4,5 };
	size_t 						i = 0;
	size_t 						length = u8.size();
	auto							size = out_u32.size();

	while(length>0)
	{
		uint32_t	lch 		= 0;
		char8_t		ch 			= u8[i];
		size_t 		codelen = (ch<0x0C0 ? 1 : trailingbytes[(ch>>2)&0x0F]+1);

		if(codelen > length)
			return -1;

		switch(codelen)
		{
			case 1 : 	lch = u8[i]; break;
			case 2 : 	lch = ((u8[i] & 0x1F) << 6)  | (u8[i+1] & 0x3F); break;
			case 3 : 	lch = ((u8[i] & 0x0F) << 12) | ((u8[i+1] & 0x3F) << 6) | (u8[i+2] & 0x3F); break;
			case 4 : 	lch = ((u8[i] & 0x07) << 18) | ((u8[i+1] & 0x3F) << 6) | ((u8[i+2] & 0x3F) << 6) | (u8[i+3] & 0x3F); break;
			default : return -1;
		}

		if(	((lch >= UNI_SUR_HIGH_START) && (lch <= UNI_SUR_LOW_END)) || (lch >= UNI_MAX_LEGAL_UTF32) )
			lch = UNI_RELACEMENT_CHAR;

		out_u32.push_back(lch);
		length -= codelen;
		i+=codelen;
 	}

	return (int)(out_u32.size() - size);
}

inline
std::optional<std::u32string>
u8_to_u32( const std::u8string & u8 )
{
	std::u32string 		u32;
	if(u8_to_u32(u8, u32) > 0)
		return u32;
	return {};
}


inline int
u32_to_u8(uint32_t u32, std::u8string & out_u8)
{
	int len=0;

	if(!(u32 & 0xFFFFFF80))
	{
		out_u8.push_back((char8_t)(u32&0x07F));
		++len;
	}
	else if(!(u32 & 0x0FFFFF800))
	{
		out_u8.push_back((char8_t)((0x0C0 | (char8_t)((u32>>6)&0x01F))));
		out_u8.push_back((char8_t)((0x080 | (char8_t)(u32&0x03F))));
		len=2;
	}
	else if(!(u32 & 0x0FFFF0000))
	{
		out_u8.push_back((char8_t)((0x0E0 | (char8_t)((u32>>12)&0x0F))));
		out_u8.push_back((char8_t)((0x080 | (char8_t)((u32>>6)&0x03F))));
		out_u8.push_back((char8_t)((0x080 | (char8_t)(u32&0x03F))));
		len=3;
	}
	else if(u32 < UNI_MAX_LEGAL_UTF32)
	{
		out_u8.push_back((char8_t)((0x0F0 | (char8_t)((u32>>18)&0x07))));
		out_u8.push_back((char8_t)((0x080 | (char8_t)((u32>>12)&0x03F))));
		out_u8.push_back((char8_t)((0x080 | (char8_t)((u32>>6)&0x03F))));
		out_u8.push_back((char8_t)((0x080 | (char8_t)(u32&0x03F))));
		len=4;
	}

	return len;
}

} // namespace adexml::unicode

#endif // ! defined GUARD_ADE_XML_UNICODE_H

