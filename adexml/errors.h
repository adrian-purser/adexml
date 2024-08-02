//=============================================================================
//	FILE:					errors.h
//	SYSTEM:				
//	DESCRIPTION:
//-----------------------------------------------------------------------------
//  COPYRIGHT:		(C) Copyright 2024 Adrian Purser. All Rights Reserved.
//	LICENCE:			MIT - See LICENSE file for details
//	MAINTAINER:		Adrian Purser <ade@adrianpurser.co.uk>
//	CREATED:			02-AUG-2024 Adrian Purser <ade@adrianpurser.co.uk>
//=============================================================================
#ifndef GUARD_ADE_XML_ERRORS_H
#define GUARD_ADE_XML_ERRORS_H

#include <system_error>

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
	ELEMENT_TAG_MISMATCH,
	INVALID_ENTITY_CHARACTER,
	UNKNOWN_ENTITY
};

std::error_code make_error_code(adexml::Error);

} // namespace adexml

namespace std {	template <> struct is_error_code_enum<adexml::Error> : true_type {}; }

#endif // ! defined GUARD_ADE_XML_ERRORS_H

