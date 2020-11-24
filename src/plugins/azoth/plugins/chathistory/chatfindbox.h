/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/gui/findnotification.h>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	class ChatFindBox : public Util::FindNotification
	{
		Q_OBJECT
	public:
		ChatFindBox (ICoreProxy_ptr, QWidget*);
	protected:
		void HandleNext (const QString& text, FindFlags flags) override;
	signals:
		void next (const QString&, ChatFindBox::FindFlags);
	};
}
}
}
