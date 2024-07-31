//=============================================================================
//	FILE:					xml_parser.cpp
//	SYSTEM:				
//	DESCRIPTION:
//-----------------------------------------------------------------------------
//  COPYRIGHT:		(C) Copyright 2024 Adrian Purser. All Rights Reserved.
//	LICENCE:			MIT - See LICENSE file for details
//	MAINTAINER:		Adrian Purser <ade@adrianpurser.co.uk>
//	CREATED:			23-JUL-2024 Adrian Purser <ade@adrianpurser.co.uk>
//=============================================================================
#include <iostream>
#include <fstream>
#include <format>
#include <cassert>
#include <cstdint>
#include "xml_parser.h"

//=============================================================================
//
//	ERRORS
//
//=============================================================================

namespace
{

struct AdeXMLErrorCategory : std::error_category
{
	const char * 	name() const noexcept override;
	std::string 	message(int ev) const override;
};


const char *
AdeXMLErrorCategory::name() const noexcept
{
	return "xml_parser";
}

std::string
AdeXMLErrorCategory::message(int ev) const
{
	switch( static_cast<adexml::Error>(ev) )
	{
		case	adexml::Error::NONE :														return "None";
		case	adexml::Error::FAILED :													return "Operation Failed";
		case 	adexml::Error::INVALID_STATE :									return "BUG: Invalid State Detected!";
		case 	adexml::Error::INVALID_ELEMENT_NAME :						return "Invalid Element Name";
		case 	adexml::Error::START_TAG_SYNTAX_ERROR :					return "Syntax Error in Start Tag";
		case 	adexml::Error::ATTRIBUTE_SYNTAX_ERROR :					return "Syntax Error in Attrbite";
		case 	adexml::Error::ATTRIBUTE_VALUE_ILLERGAL_CHAR :	return "Illegal character in attribute value";
		case 	adexml::Error::ATTRIBUTE_DUPLICATE_NAME :				return "Duplicate Attribute Name";
		case 	adexml::Error::ELEMENT_TAG_MISMATCH :						return "Element Tag Mismatch";
	}
	return "(Unknown Error!)";
}

const AdeXMLErrorCategory sg_adexml_error_category;

} // namespace

namespace adexml
{
	std::error_code make_error_code(adexml::Error e)
	{
		return {static_cast<int>(e), sg_adexml_error_category};
	}
} // namespace adexml


//=============================================================================
//
//	XML PARSER
//
//=============================================================================

namespace adexml
{

std::error_code
Parser::write(std::span<char8_t> data)
{
	std::error_code ec;

	for(auto ch : data)
	{
		switch(m_encoding)
		{
			case Parser::ENCODING_PLAIN_TEXT : 	ec = put(static_cast<char32_t>(ch)); break;
			case Parser::ENCODING_UTF8 : 				m_u8_parser.put(ch).and_then([&](char32_t u32)->std::optional<char32_t> {ec = put(u32); return std::nullopt;}); break;
			default :														ec = std::make_error_code(std::errc::protocol_not_supported); break;
		}
		if(ec)
			break;
	}

	return ec;
}

//-----------------------------------------------------------------------------
//	STATE: IDLE
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_idle(char32_t ch)
{
	switch(ch)
	{
		case adexml::LESS_THAN_SIGN :
			set_state(State::STATE_TAG_START);
			m_tag_name.clear();
			m_tag_namespace.clear();
			m_attr_name.clear();
			m_element_type = ElementType::ELEMENT;
			break;

		case adexml::CARRIAGE_RETURN : [[fallthrough]];
		case adexml::LINE_FEED :
			break;

		case adexml::SPACE :	[[fallthrough]];
		case adexml::CHARACTER_TABULATION :
			if(!m_element_stack.empty())
			{
				auto & element = m_element_stack.back();
				if(!element.content.empty())
					adexml::unicode::u32_to_u8(ch,element.content);
			}
			break;

		default :
			if(!m_element_stack.empty())
				adexml::unicode::u32_to_u8(ch,m_element_stack.back().content);
			break;
	}
	return {};
}

//-----------------------------------------------------------------------------
//	STATE: TAG_START
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_tag_start(char32_t ch)
{
	switch(ch)
	{
		case adexml::SOLIDUS :
			set_state(State::STATE_END_TAG_NAME);
			break;

		case adexml::QUESTION_MARK :
			m_element_type = ElementType::PI;
			break;

		case adexml::SPACE :	[[fallthrough]];
		case adexml::CHARACTER_TABULATION :
			break;

		default :
			if(is_name_start_char(ch))
			{
				adexml::unicode::u32_to_u8(ch,m_tag_name);
				set_state(State::STATE_START_TAG_NAME);
				m_element_stack.push_back({});
				m_element_stack.back().type = m_element_type;
			}
			else
			{
				set_state(State::STATE_ERROR);
				return adexml::Error::INVALID_ELEMENT_NAME;
			}
			break;
	}
	return {};
}

//-----------------------------------------------------------------------------
//	STATE: START_TAG_NAME
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_start_tag_name(char32_t ch)
{
//	assert(!m_element_stack.empty());
//	auto & element = m_element_stack.back();

	if(is_name_char(ch))
		adexml::unicode::u32_to_u8(ch,m_tag_name);
	else 
		switch(ch)
		{
			case adexml::GREATER_THAN_SIGN :
				on_end_start_tag();
				break;

			case adexml::SPACE :	[[fallthrough]];
			case adexml::CHARACTER_TABULATION :
				set_state(State::STATE_START_TAG_BODY);
				break;

			default:
				set_state(State::STATE_ERROR);
				return adexml::Error::INVALID_ELEMENT_NAME;
				break;
		}

	return {};
}

//-----------------------------------------------------------------------------
//	STATE: START_TAG_BODY
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_start_tag_body(char32_t ch)
{
	assert(!m_element_stack.empty());
	auto & element = m_element_stack.back();

	switch(ch)
	{
		//-------------------------------------------------------------------------
		case adexml::GREATER_THAN_SIGN :
		//-------------------------------------------------------------------------
			on_end_start_tag();
			break;

		//-------------------------------------------------------------------------
		case adexml::SOLIDUS :
		//-------------------------------------------------------------------------
			if(element.type == ElementType::ELEMENT)
				set_state(State::STATE_START_TAG_CLOSE);
			else
			{
				set_state(State::STATE_ERROR);
				return adexml::Error::ATTRIBUTE_SYNTAX_ERROR;
			}
			break;

		//-------------------------------------------------------------------------
		case adexml::EQUALS_SIGN :
		//-------------------------------------------------------------------------
			if(m_attr_name.empty())
			{
				set_state(State::STATE_ERROR);
				return adexml::Error::ATTRIBUTE_SYNTAX_ERROR;
			}
			set_state(State::STATE_ATTRIBUTE_EXPECT_VALUE);
			break;

		//-------------------------------------------------------------------------
		case adexml::QUESTION_MARK :
		//-------------------------------------------------------------------------
			if(element.type == ElementType::PI)
				set_state(State::STATE_END_PI_TAG);
			else
			{
				set_state(State::STATE_ERROR);
				return adexml::Error::ATTRIBUTE_SYNTAX_ERROR;
			}
			break;

		//-------------------------------------------------------------------------
		default :
		//-------------------------------------------------------------------------
			if(is_name_start_char(ch))
			{
				if(!m_attr_name.empty())
				{
					set_state(State::STATE_ERROR);
					return adexml::Error::ATTRIBUTE_SYNTAX_ERROR;
				}

				adexml::unicode::u32_to_u8(ch,m_attr_name);
				set_state(State::STATE_ATTRIBUTE_NAME);
			}
			break;
	}
	
	return {};
}

//-----------------------------------------------------------------------------
//	STATE: START_TAG_CLOSE
//-----------------------------------------------------------------------------
std::error_code
Parser::do_state_start_tag_close(char32_t ch)
{
	assert(!m_element_stack.empty());
	auto & element = m_element_stack.back();

	if(ch == adexml::GREATER_THAN_SIGN)
	{
		element.b_closed = true;
		on_end_start_tag();
	}
	else
	{
		set_state(State::STATE_ERROR);
		return adexml::Error::START_TAG_SYNTAX_ERROR;
	}
	return {};
}

//-----------------------------------------------------------------------------
//	STATE: END_TAG
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_end_tag(char32_t ch)
{
	if(is_name_start_char(ch))
	{
		m_tag_name.clear();
		adexml::unicode::u32_to_u8(ch,m_tag_name);
		set_state(State::STATE_END_TAG_NAME);
	}
	else 
		switch(ch)
		{
			case adexml::SPACE :	[[fallthrough]];
			case adexml::CHARACTER_TABULATION :
				break;

			default:
				set_state(State::STATE_ERROR);
				return adexml::Error::INVALID_ELEMENT_NAME;
		}

	return {};
}

//-----------------------------------------------------------------------------
//	STATE: END_TAG_NAME
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_end_tag_name(char32_t ch)
{

	if(is_name_char(ch))
		adexml::unicode::u32_to_u8(ch,m_tag_name);
	else 
		switch(ch)
		{
			case adexml::GREATER_THAN_SIGN :
				return on_end_end_tag();

			case adexml::SPACE :	[[fallthrough]];
			case adexml::CHARACTER_TABULATION :
				set_state(State::STATE_END_TAG_BODY);
				break;

			default:
				set_state(State::STATE_ERROR);
				return adexml::Error::INVALID_ELEMENT_NAME;
		}

	return {};
}

//-----------------------------------------------------------------------------
//	STATE: END_TAG_NAME
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_end_tag_body(char32_t ch)
{
	switch(ch)
	{
		case adexml::GREATER_THAN_SIGN :
			return on_end_end_tag();

		case adexml::SPACE :	[[fallthrough]];
		case adexml::CHARACTER_TABULATION :
			break;

		default:
			set_state(State::STATE_ERROR);
			return adexml::Error::INVALID_ELEMENT_NAME;
	}
	return {};
}

//-----------------------------------------------------------------------------
//	STATE: END_PI_TAG
//-----------------------------------------------------------------------------

std::error_code
Parser::do_state_end_pi_tag(char32_t ch)
{
	switch(ch)
	{
		case adexml::GREATER_THAN_SIGN :
			return on_pi_tag();

		default:
			set_state(State::STATE_ERROR);
			return adexml::Error::INVALID_ELEMENT_NAME;
	}
	return {};
}




//-----------------------------------------------------------------------------
//	STATE: ATTRIBUTE_NAME
//-----------------------------------------------------------------------------
std::error_code
Parser::do_state_attribute_name(char32_t ch)
{
	if(is_name_char(ch))
		adexml::unicode::u32_to_u8(ch,m_attr_name);
	else
		switch(ch)
		{
			case adexml::EQUALS_SIGN :
				set_state(State::STATE_ATTRIBUTE_EXPECT_VALUE);
				break;

			case adexml::SPACE :	[[fallthrough]];
			case adexml::CHARACTER_TABULATION :
				set_state(State::STATE_START_TAG_BODY);
				break;

			default :
				set_state(State::STATE_ERROR);
				return adexml::Error::ATTRIBUTE_SYNTAX_ERROR;
		}
	return {};
}

//-----------------------------------------------------------------------------
//	STATE: ATTRIBUTE_EXPECT_VALUE
//-----------------------------------------------------------------------------
std::error_code
Parser::do_state_attribute_expect_value(char32_t ch)
{
	switch(ch)
	{
		case adexml::SPACE :	[[fallthrough]];
		case adexml::CHARACTER_TABULATION :
			break;

		case adexml::QUOTATION_MARK :
		case adexml::APOSTROPHE :
			set_state(State::STATE_ATTRIBUTE_VALUE);
			m_attr_delimeter = ch;
			break;

		default :
			set_state(State::STATE_ERROR);
			return adexml::Error::ATTRIBUTE_SYNTAX_ERROR;
	}
	return {};
}


//-----------------------------------------------------------------------------
//	STATE: ATTRIBUTE_VALUE
//-----------------------------------------------------------------------------
std::error_code
Parser::do_state_attribute_value(char32_t ch)
{
	if(ch==m_attr_delimeter)
	{
		//TODO(Ade): Add Attribute
		assert(!m_element_stack.empty());
		auto & element = m_element_stack.back();
		if(element.attributes.count(m_attr_name) > 0)
		{
			set_state(State::STATE_ERROR);
			return adexml::Error::ATTRIBUTE_DUPLICATE_NAME;
		}
		
		element.attributes[m_attr_name] = m_attr_value;
		m_attr_name.clear();
		m_attr_value.clear();
		set_state(State::STATE_START_TAG_BODY);
	}
	else if((ch == adexml::LESS_THAN_SIGN) || (ch == adexml::AMPERSAND))
	{
		set_state(State::STATE_ERROR);
		return adexml::Error::ATTRIBUTE_VALUE_ILLERGAL_CHAR;
	}
	else
	{
		adexml::unicode::u32_to_u8(ch,m_attr_value);
	}
	return {};
}

std::error_code
Parser::put(char32_t ch)
{
//	std::cout << std::format("U32: 0x{:08X}\n", (uint32_t)ch);
	/*
	switch(ch)
	{
		case '\r' :
		case '\n' :
			if(!m_b_double_quotes && !m_b_double_quotes)
				return {};
			break;

		case adexml::SPACE :
		case adexml::CHARACTER_TABULATION :
			if(!m_b_double_quotes && !m_b_double_quotes)
			{
				ch == adexml::SPACE;
				if(m_last_char == ch)
					return {};
			}
			break;

		case adexml::APOSTROPHE :
			if(!m_b_double_quotes)
				m_b_single_quotes = !m_b_single_quotes;
			break;

		case adexml::QUOTATION_MARK :
			if(!m_b_single_quotes)
				m_b_double_quotes = !m_b_double_quotes;
			break;
	}
*/
	switch(m_state)
	{
		case State::STATE_ERROR:										return adexml::Error::FAILED;
		case State::STATE_IDLE: 										return do_state_idle(ch);
		case State::STATE_TAG_START: 								return do_state_tag_start(ch);
		case State::STATE_START_TAG_NAME: 					return do_state_start_tag_name(ch);
		case State::STATE_START_TAG_BODY: 					return do_state_start_tag_body(ch);
		case State::STATE_START_TAG_CLOSE:					return do_state_start_tag_close(ch);
		case State::STATE_END_TAG:									return do_state_end_tag(ch);
		case State::STATE_END_TAG_NAME:							return do_state_end_tag_name(ch);
		case State::STATE_END_TAG_BODY:							return do_state_end_tag_body(ch);
		case State::STATE_END_PI_TAG:								return do_state_end_pi_tag(ch);
		case State::STATE_ATTRIBUTE_NAME:						return do_state_attribute_name(ch);
		case State::STATE_ATTRIBUTE_EXPECT_VALUE:		return do_state_attribute_expect_value(ch);
		case State::STATE_ATTRIBUTE_VALUE:					return do_state_attribute_value(ch);
		default:														set_state(State::STATE_ERROR); return adexml::Error::INVALID_STATE;
	}

	return {};
}

void
Parser::build_path_string()
{
	m_stack_path.clear();
	for(auto & el : m_element_stack)
	{
		if(!m_stack_path.empty())
			m_stack_path.push_back('/');
		m_stack_path.append(el.name);
	}
};


void
Parser::on_end_start_tag()
{
	set_state(State::STATE_IDLE);
	assert(!m_element_stack.empty());
	auto & element 			= m_element_stack.back();
	element.name_space	= m_tag_namespace;
	element.name 				= m_tag_name;
	if(!m_stack_path.empty())
		m_stack_path.push_back('/');
	m_stack_path.append(m_tag_name);

/*
	std::cout << "START_TAG: ";
	for(auto ch : m_tag_namespace) std::cout.put(ch);
	if(!m_tag_namespace.empty())
		std::cout.put(':');
	for(auto ch : m_tag_name)	std::cout.put(ch);

	if(element.b_closed)
		std::cout << " <end>";

	std::cout << " path: ";
	for(auto ch : m_stack_path)	std::cout.put(ch);

	std::cout.put('\n');

	for(auto & [name, value] : element.attributes)
	{
		std::cout << "    ATTR: ";
		for(auto ch : name)		std::cout.put(ch);
		std::cout << " = \"";
		for(auto ch : value)		std::cout.put(ch);
		std::cout << "\"\n";
	}
*/

	if(m_callback)
		m_callback(ACTION_START_ELEMENT, m_stack_path, m_element_stack);

	if(element.b_closed)
	{
		m_element_stack.pop_back();
		build_path_string();
	}

}

std::error_code
Parser::on_end_end_tag()
{
	set_state(State::STATE_IDLE);
	assert(!m_element_stack.empty());
	auto & element 			= m_element_stack.back();

	//---------------------------------------------------------------------------
	// Check whether the start and end tag names match
	//---------------------------------------------------------------------------
	if(element.name != m_tag_name)
	{
		std::cout << "element.name: '";
		for(auto ch : element.name) std::cout.put(ch);
		std::cout << "' m_tag_name: '";
		for(auto ch : m_tag_name) std::cout.put(ch);
		std::cout << "'\n";

		set_state(State::STATE_ERROR);
		return adexml::Error::ELEMENT_TAG_MISMATCH;
	}

	//---------------------------------------------------------------------------
	//	Report the end tag by calling the user provided callback.
	//---------------------------------------------------------------------------
	//TODO(Ade): Callback to the user
//	std::cout << "CONTENT:   '";
//	for(auto ch : element.content) std::cout.put(ch);
//	std::cout << "'\n";

	if(m_callback)
		m_callback(ACTION_END_ELEMENT, m_stack_path, m_element_stack);

	//---------------------------------------------------------------------------
	//	Remove the element from the element stack.
	//---------------------------------------------------------------------------
	m_element_stack.pop_back();
	build_path_string();

/*
	std::cout << "END_TAG:   ";
	for(auto ch : m_tag_namespace)
		std::cout.put(ch);
	if(!m_tag_namespace.empty())
		std::cout.put(':');
	for(auto ch : m_tag_name)
		std::cout.put(ch);
	std::cout.put('\n');
*/
	return {};
}

std::error_code
Parser::on_pi_tag()
{
	set_state(State::STATE_IDLE);
	assert(!m_element_stack.empty());
	auto & element 			= m_element_stack.back();
	element.name_space	= m_tag_namespace;
	element.name 				= m_tag_name;

	std::cout << "PI: ";
	for(auto ch : element.name)	std::cout.put(ch);
	std::cout.put('\n');

	for(auto & [name, value] : element.attributes)
	{
		std::cout << "    ATTR: ";
		for(auto ch : name)		std::cout.put(ch);
		std::cout << " = \"";
		for(auto ch : value)		std::cout.put(ch);
		std::cout << "\"\n";
	}

	//---------------------------------------------------------------------------
	//	Remove the element from the element stack.
	//---------------------------------------------------------------------------
	m_element_stack.pop_back();
	build_path_string();

	return {};
}



} // namespace adexml


