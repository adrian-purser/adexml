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
#include <expected>
#include <optional>
#include <system_error>
#include "unicode.h"
#include "entity.h"

namespace adexml
{

//=============================================================================
//
//	ELEMENT
//
//=============================================================================

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

//=============================================================================
//
//	XML PARSER
//
//=============================================================================

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
	adexml::unicode::U8Parser			m_u8_parser;
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
	EntityParser									m_entity_parser;

public:
	Parser() = delete;
	Parser(Callback callback)	: m_callback(callback) {}
	~Parser() = default;

	std::error_code				write(std::span<char8_t> data);
	std::error_code				put(char8_t	ch)										{return write({&ch,1U});}
	std::error_code				put(char32_t ch);

private:
	void									set_state(State state);

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

#endif // ! defined GUARD_ADE_XML_PARSER_H

