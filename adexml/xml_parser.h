//=============================================================================
//	FILE:					xml_parser.h
//	SYSTEM:				
//	DESCRIPTION:
//-----------------------------------------------------------------------------
//  COPYRIGHT:		(C) Copyright 2024 Adrian Purser. All Rights Reserved.
//	LICENCE:			MIT - See LICENSE file for details
//	MAINTAINER:		Adrian Purser <ade@adrianpurser.co.uk>
//	CREATED:			23-JUL-2024 Adrian Purser <ade@adrianpurser.co.uk>
//=============================================================================
//	NOTES:
//		https://www.w3.org/TR/REC-xml/
//=============================================================================

#ifndef GUARD_ADE_XML_PARSER_H
#define GUARD_ADE_XML_PARSER_H

#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <functional>
#include <system_error>
#include "utility/unicode.h"

namespace adexml
{
enum class Error
{
	NONE = 0,

	FAILED,
	INVALID_STATE,
	INVALID_ELEMENT_NAME,
	START_TAG_SYNTAX_ERROR,
	ATTRIBUTE_SYNTAX_ERROR,
	ATTRIBUTE_VALUE_ILLERGAL_CHAR,
	ATTRIBUTE_DUPLICATE_NAME,
	ELEMENT_TAG_MISMATCH
};

std::error_code make_error_code(adexml::Error);

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

enum class ElementType
{
	ELEMENT,
	PI,
	DTD
};

struct Element
{
	std::u8string																			name_space;
	std::u8string																			name;
	std::unordered_map<std::u8string, std::u8string>	attributes;
	std::u8string																			content;
	ElementType																				type				= ElementType::ELEMENT;
	bool																							b_closed 		= false;

	bool																	has_attribute(const std::u8string_view attr_mame) const	
																				{
																					auto ifind=attributes.find(std::u8string(attr_mame)); 
																					return ifind != attributes.end();
																				}
																				
	const std::optional<std::u8string> 		attribute(const std::u8string_view attr_mame) const			
																				{
																					auto ifind=attributes.find(std::u8string(attr_mame)); 
																					if(ifind != attributes.end()) 
																						return ifind->second; 
																					return {};
																				} 
};


class Parser
{
public:
	enum Action
	{
		ACTION_START_ELEMENT,
		ACTION_END_ELEMENT,
		ACTION_PI
	};

	enum Encoding 
	{
		ENCODING_PLAIN_TEXT,
		ENCODING_UTF8,
		ENCODING_UTF16_LITTLE_ENDIAN,
		ENCODING_UTF16_BIG_ENDIAN,
		ENCODING_UTF32_LITTLE_ENDIAN,
		ENCODING_UTF32_BIG_ENDIAN
	};

	using Callback = std::function<std::error_code (	adexml::Parser::Action 								action,
																										const std::u8string &									path,
																										const std::vector<adexml::Element> & 	element_stack )>;

private:
	enum State
	{
		STATE_IDLE,
		STATE_ERROR,
		STATE_TAG_START,
		STATE_START_TAG_NAME,
		STATE_START_TAG_BODY,
		STATE_START_TAG_CLOSE,
		STATE_END_TAG,
		STATE_END_TAG_NAME,
		STATE_END_TAG_BODY,
		STATE_END_PI_TAG,
		STATE_ATTRIBUTE_NAME,
		STATE_ATTRIBUTE_EXPECT_VALUE,
		STATE_ATTRIBUTE_VALUE,
		STATE_ATTRIBUTE_VALUE_STRING
	};

	Callback											m_callback;
	std::vector<Element>					m_element_stack;

	Encoding											m_encoding = ENCODING_UTF8;
	ade::unicode::U8Parser				m_u8_parser;
	std::u32string								m_buffer_u32;
	std::u8string									m_tag_name;
	std::u8string									m_tag_namespace;
	std::u8string									m_attr_name;
	std::u8string									m_attr_value;
	std::u8string									m_stack_path;
	State													m_state = STATE_IDLE;
	char32_t											m_last_char = 0;
	char32_t 											m_attr_delimeter = 0;
	ElementType										m_element_type = ElementType::ELEMENT;

public:
	Parser() = delete;
	Parser(Callback callback)	: m_callback(callback) {}
	~Parser() = default;

	std::error_code				write(std::span<char8_t> data);
	std::error_code				put(char8_t	ch)										{return write({&ch,1U});}
	std::error_code				put(char32_t ch);

private:
	void									set_state(State state)						{m_state = state;}
	void									on_start_tag();
	void									on_end_start_tag();
	std::error_code				on_end_end_tag();
	std::error_code				on_pi_tag();

	std::error_code				do_state_idle(char32_t ch);
	std::error_code				do_state_tag_start(char32_t ch);
	std::error_code				do_state_start_tag_name(char32_t ch);
	std::error_code				do_state_start_tag_body(char32_t ch);
	std::error_code				do_state_start_tag_close(char32_t ch);
	std::error_code				do_state_end_tag(char32_t ch);
	std::error_code				do_state_end_tag_name(char32_t ch);
	std::error_code				do_state_end_tag_body(char32_t ch);
	std::error_code				do_state_end_pi_tag(char32_t ch);
	std::error_code				do_state_attribute_name(char32_t ch);
	std::error_code				do_state_attribute_expect_value(char32_t ch);
	std::error_code				do_state_attribute_value(char32_t ch);

	void									build_path_string();

	static constexpr bool	is_name_start_char(char32_t ch)	{	return 	(ch==adexml::COLON) || (ch==adexml::LOW_LINE) || isalpha(ch) ||
																																	((ch>=0xC0) && (ch<=0xD6)) ||
																																	((ch>=0xD8) && (ch<=0xF6)) ||
																																	((ch>=0xF8) && (ch<=0x02FF)) ||
																																	((ch>=0x0370) && (ch<=0x037D)) ||
																																	((ch>=0x037F) && (ch<=0x1FFF)) ||
																																	((ch>=0x200C) && (ch<=0x200D)) ||
																																	((ch>=0x2070) && (ch<=0x218F)) ||
																																	((ch>=0x2C00) && (ch<=0x2FEF)) ||
																																	((ch>=0x3001) && (ch<=0xD7FF)) ||
																																	((ch>=0xF900) && (ch<=0xFDCF)) ||
																																	((ch>=0xFDF0) && (ch<=0xFFFD)) ||
																																	((ch>=0x10000) && (ch<=0xEFFFF)); }

	static constexpr bool is_name_char(char32_t ch)				{	return 	is_name_start_char(ch) || 
																																	(ch==adexml::HYPHEN_MINUS) || (ch==adexml::FULL_STOP) ||
																																	isdigit(ch) || (ch==0xB7) ||
																																	((ch>=0x0300) && (ch<=0x036F)) ||
																																	((ch>=0x203F) && (ch<=0x2040)); }
																																	
};




} // namespace adexml


namespace std {	template <> struct is_error_code_enum<adexml::Error> : true_type {}; }

#endif // ! defined GUARD_ADE_XML_PARSER_H

