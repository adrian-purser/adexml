//=============================================================================
//	FILE:					errors.cpp
//	SYSTEM:				
//	DESCRIPTION:
//-----------------------------------------------------------------------------
//  COPYRIGHT:		(C) Copyright 2024 Adrian Purser. All Rights Reserved.
//	LICENCE:			MIT - See LICENSE file for details
//	MAINTAINER:		Adrian Purser <ade@adrianpurser.co.uk>
//	CREATED:			02-AUG-2024 Adrian Purser <ade@adrianpurser.co.uk>
//=============================================================================

#include "errors.h"

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
		case	adexml::Error::INVALID_ENTITY_CHARACTER :				return "Invalid Entity Character";
		case 	adexml::Error::UNKNOWN_ENTITY :									return "Unknown Entity";
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
