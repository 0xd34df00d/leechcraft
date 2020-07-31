/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class InvalidPacket : public std::runtime_error
	{
	public:
		explicit InvalidPacket (const std::string&);
	};

	class TooShortBA : public std::runtime_error
	{
	public:
		explicit TooShortBA (const std::string&);
	};

	class MsgParseError : public std::runtime_error
	{
	public:
		explicit MsgParseError (const std::string&);
	};
}
}
}
}
