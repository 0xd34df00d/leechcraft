/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QKeyEvent;

namespace LC::Util
{
	class ShortcutManager;
}

namespace LC::Eleeminator
{
	class ShortcutPasser : public QObject
	{
		bool PassNextShortcut_ = false;
	public:
		explicit ShortcutPasser (Util::ShortcutManager&, QWidget&);
	private:
		bool ShouldPassShortcut (const QKeyEvent&) const;
	};
}
