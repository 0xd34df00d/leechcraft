/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QXmppMessage;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
namespace Xep0334
{
	enum class MessageHint
	{
		NoPermStorage,
		NoStorage,
		NoCopies
	};

	void SetHint (QXmppMessage&, MessageHint);
}
}
}
}
