/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chatfindbox.h"

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	ChatFindBox::ChatFindBox (ICoreProxy_ptr proxy, QWidget *parent)
	: FindNotification { proxy, parent }
	{
	}

	void ChatFindBox::HandleNext (const QString& text, FindFlags flags)
	{
		emit next (text, flags);
	}
}
}
}
