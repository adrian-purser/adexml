//=============================================================================
//	FILE:					entity.h
//	SYSTEM:				
//	DESCRIPTION:
//-----------------------------------------------------------------------------
//  COPYRIGHT:		(C) Copyright 2024 Adrian Purser. All Rights Reserved.
//	LICENCE:			MIT - See LICENSE file for details
//	MAINTAINER:		Adrian Purser <ade@adrianpurser.co.uk>
//	CREATED:			02-AUG-2024 Adrian Purser <ade@adrianpurser.co.uk>
//=============================================================================
//	NOTES:
//		https://www.w3.org/TR/REC-xml/
//=============================================================================

#ifndef GUARD_ADE_XML_ENTITY_H
#define GUARD_ADE_XML_ENTITY_H

#include "errors.h"

namespace adexml
{

enum
{
	CHARACTER_TABULATION		= 0x0009,
	LINE_FEED								= 0x000A,
	FORM_FEED								= 0x000C,
	CARRIAGE_RETURN					= 0x000D,
	SPACE										= 0x0020,
	EXCLAMATION_MARK				= 0x0021,
	QUOTATION_MARK					= 0x0022,
	NUMBER_SIGN							= 0x0023,
	AMPERSAND								= 0x0026,
	APOSTROPHE							= 0x0027,
	HYPHEN_MINUS						= 0x002D,
	FULL_STOP								= 0x002E,
	SOLIDUS									= 0x002F,

	DIGIT_ZERO							= 0x0030,
	DIGIT_ONE								= 0x0031,
	DIGIT_TWO								= 0x0032,
	DIGIT_THREE							= 0x0033,
	DIGIT_FOUR							= 0x0034,
	DIGIT_FIVE							= 0x0035,
	DIGIT_SIX								= 0x0036,
	DIGIT_SEVEN							= 0x0037,
	DIGIT_EIGHT							= 0x0038,
	DIGIT_NINE							= 0x0039,

	COLON										= 0x003A,
	SEMICOLON								= 0x003B,
	LESS_THAN_SIGN					= 0x003C,
	EQUALS_SIGN							= 0x003D,
	GREATER_THAN_SIGN				= 0x003E,
	QUESTION_MARK						= 0x003F,

	LATIN_CAPITAL_LETTER_A	= 0x0041,
	LATIN_CAPITAL_LETTER_B	= 0x0042,
	LATIN_CAPITAL_LETTER_C	= 0x0043,
	LATIN_CAPITAL_LETTER_D	= 0x0044,
	LATIN_CAPITAL_LETTER_E	= 0x0045,
	LATIN_CAPITAL_LETTER_F	= 0x0046,
	LATIN_CAPITAL_LETTER_G	= 0x0047,
	LATIN_CAPITAL_LETTER_H	= 0x0048,
	LATIN_CAPITAL_LETTER_I	= 0x0049,
	LATIN_CAPITAL_LETTER_J	= 0x004A,
	LATIN_CAPITAL_LETTER_K	= 0x004B,
	LATIN_CAPITAL_LETTER_L	= 0x004C,
	LATIN_CAPITAL_LETTER_M	= 0x004D,
	LATIN_CAPITAL_LETTER_N	= 0x004E,
	LATIN_CAPITAL_LETTER_O	= 0x004F,
	LATIN_CAPITAL_LETTER_P	= 0x0050,
	LATIN_CAPITAL_LETTER_Q	= 0x0051,
	LATIN_CAPITAL_LETTER_R	= 0x0052,
	LATIN_CAPITAL_LETTER_S	= 0x0053,
	LATIN_CAPITAL_LETTER_T	= 0x0054,
	LATIN_CAPITAL_LETTER_U	= 0x0055,
	LATIN_CAPITAL_LETTER_V	= 0x0056,
	LATIN_CAPITAL_LETTER_W	= 0x0057,
	LATIN_CAPITAL_LETTER_X	= 0x0058,
	LATIN_CAPITAL_LETTER_Y	= 0x0059,
	LATIN_CAPITAL_LETTER_Z	= 0x005A,

	SQUARE_BRACKET_LEFT			= 0x005B,
	REVERSE_SOLIDUS					= 0x005C,
	SQUARE_BRACKET_RIGHT		= 0x005D,
	LOW_LINE								= 0x005F,
	GRAVE_ACCENT						= 0x0060,

	LATIN_SMALL_LETTER_A		= 0x0061,
	LATIN_SMALL_LETTER_B		= 0x0062,
	LATIN_SMALL_LETTER_C		= 0x0063,
	LATIN_SMALL_LETTER_D		= 0x0064,
	LATIN_SMALL_LETTER_E		= 0x0065,
	LATIN_SMALL_LETTER_F		= 0x0066,
	LATIN_SMALL_LETTER_G		= 0x0067,
	LATIN_SMALL_LETTER_H		= 0x0068,
	LATIN_SMALL_LETTER_I		= 0x0069,
	LATIN_SMALL_LETTER_J		= 0x006A,
	LATIN_SMALL_LETTER_K		= 0x006B,
	LATIN_SMALL_LETTER_L		= 0x006C,
	LATIN_SMALL_LETTER_M		= 0x006D,
	LATIN_SMALL_LETTER_N		= 0x006E,
	LATIN_SMALL_LETTER_O		= 0x006F,
	LATIN_SMALL_LETTER_P		= 0x0070,
	LATIN_SMALL_LETTER_Q		= 0x0071,
	LATIN_SMALL_LETTER_R		= 0x0072,
	LATIN_SMALL_LETTER_S		= 0x0073,
	LATIN_SMALL_LETTER_T		= 0x0074,
	LATIN_SMALL_LETTER_U		= 0x0075,
	LATIN_SMALL_LETTER_V		= 0x0076,
	LATIN_SMALL_LETTER_W		= 0x0077,
	LATIN_SMALL_LETTER_X		= 0x0078,
	LATIN_SMALL_LETTER_Y		= 0x0079,
	LATIN_SMALL_LETTER_Z		= 0x007A,

	REPLACEMENT_CHARACTER		= 0xFFFD,

	END_OF_FILE							= std::char_traits<char16_t>::eof()
};


class EntityParser
{
private:
	enum State
	{
		STATE_IDLE,
		STATE_START,
		STATE_NUMERIC,
		STATE_NAME
	};

	std::string			m_name;
	char32_t				m_code 	= 0;
	State						m_state = STATE_IDLE;

public:
	void		reset() {m_state = STATE_IDLE;}

	std::error_code
	parse(char32_t ch, std::function<void(char32_t)> callback)
	{
		switch(m_state)
		{
			//-------------------------------
			case STATE_IDLE :
			//-------------------------------
				if(ch == AMPERSAND)
					m_state = STATE_START;
				else
					callback(ch);
				break;

			//-------------------------------
			case STATE_START :
			//-------------------------------
				if(ch == NUMBER_SIGN)
				{
					m_state = STATE_NUMERIC;
					m_code = 0;
				}
				else if(isalpha(ch))
				{
					m_state = STATE_NAME;
					m_name.clear();
					m_name.push_back(ch);
				}
				else
				{
					m_state = STATE_IDLE;
					return make_error_code(adexml::Error::INVALID_ENTITY_CHARACTER);
				}
				break;

			//-------------------------------
			case STATE_NUMERIC :
			//-------------------------------
				if(ch == SEMICOLON)
				{
					m_state = STATE_IDLE;
					callback(m_code);
				}
				if(isxdigit(ch))
					m_code = (m_code << 4) | ((ch <= '9') ? ch - '0' : (toupper(ch)-'A')+10);
				else
				{
					m_state = STATE_IDLE;
					return make_error_code(adexml::Error::INVALID_ENTITY_CHARACTER);
				}
				break;
			
			//-------------------------------
			case STATE_NAME :
			//-------------------------------
				if(ch == SEMICOLON)
				{
					m_state = STATE_IDLE;
					if(m_name == "amp")				callback(AMPERSAND); 
					else if(m_name == "apos")	callback(APOSTROPHE);
					else if(m_name == "quot")	callback(QUOTATION_MARK);
					else if(m_name == "lt")		callback(LESS_THAN_SIGN);
					else if(m_name == "gt")		callback(GREATER_THAN_SIGN);
					else return make_error_code(adexml::Error::UNKNOWN_ENTITY);
				}
				else if(isalnum(ch))
					m_name.push_back(ch);
				else
				{
					m_state = STATE_IDLE;
					return make_error_code(adexml::Error::INVALID_ENTITY_CHARACTER);
				}
				break;

			default:
				m_state = STATE_IDLE;
				return make_error_code(adexml::Error::INVALID_ENTITY_CHARACTER);
		}

		return {};
	}

};

} // namespace adexml

#endif // ! defined GUARD_ADE_XML_ENTITY_H

