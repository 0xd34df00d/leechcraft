/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shortcutpasser.h"
#include <QKeyEvent>
#include <QShortcut>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/lambdaeventfilter.h>
#include "termtab.h"

namespace LC::Eleeminator
{
	namespace
	{
		bool IsModifierKey (int key)
		{
			switch (key)
			{
			case Qt::Key_Control:
			case Qt::Key_Shift:
			case Qt::Key_Alt:
			case Qt::Key_AltGr:
			case Qt::Key_Meta:
				return true;
			default:
				return false;
			}
		}
	}

	ShortcutPasser::ShortcutPasser (Util::ShortcutManager& scMgr, QWidget& tab)
	: QObject { &tab }
	{
		const auto passShortcutSc = new QShortcut { {}, &tab, this, [this] { PassNextShortcut_ = true; } };
		scMgr.RegisterShortcut ("org.LeechCraft.Eleeminator.PassShortcut", {}, passShortcutSc);

		tab.installEventFilter (Util::MakeLambdaEventFilter<QEvent::ShortcutOverride> ([this] (QKeyEvent *ev)
				{
					if (!ShouldPassShortcut (*ev))
					{
						ev->accept ();
						return false;
					}
					if (!IsModifierKey (ev->key ()))
						PassNextShortcut_ = false;
					return false;
				},
				*this));
	}

	bool ShortcutPasser::ShouldPassShortcut (const QKeyEvent& kev) const
	{
		if (PassNextShortcut_)
			return true;

		const auto mods = kev.modifiers ();
		const bool isCtrlShiftShortcut = mods & Qt::ControlModifier && mods & Qt::ShiftModifier;
		return isCtrlShiftShortcut;
	}
}
