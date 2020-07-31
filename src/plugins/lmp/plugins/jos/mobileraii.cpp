/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mobileraii.h"

namespace LC
{
namespace LMP
{
namespace jOS
{
	MobileRaiiException::MobileRaiiException (const std::string& str, uint16_t err)
	: runtime_error { str }
	, ErrCode_ { err }
	{
	}

	MobileRaiiException::~MobileRaiiException () noexcept
	{
	}

	uint16_t MobileRaiiException::GetErrCode () const
	{
		return ErrCode_;
	}
}
}
}
