/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "isupportmicroblogs.h"

namespace LC
{
namespace Azoth
{
	class IHaveMicroblogs
	{
	public:
		virtual ~IHaveMicroblogs () {}

		virtual void SubmitPost (const Post&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveMicroblogs,
		"org.Deviant.LeechCraft.Azoth.IHaveMicroblogs/1.0")
