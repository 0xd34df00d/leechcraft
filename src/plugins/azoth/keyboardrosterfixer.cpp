/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "keyboardrosterfixer.h"
#include <QKeyEvent>
#include <QApplication>
#include <QTreeView>
#include <QLineEdit>

namespace LC
{
namespace Azoth
{
	KeyboardRosterFixer::KeyboardRosterFixer (QLineEdit *edit, QTreeView *view, QObject *parent)
	: QObject (parent)
	, Edit_ (edit)
	, View_ (view)
	{
		Edit_->installEventFilter (this);
	}

	void KeyboardRosterFixer::SetInterceptEnter (bool intercept)
	{
		InterceptEnter_ = intercept;
	}

	bool KeyboardRosterFixer::eventFilter (QObject*, QEvent *e)
	{
		if (e->type () != QEvent::KeyPress &&
			e->type () != QEvent::KeyRelease)
			return false;

		if (IsSearching_ &&
				Edit_->text ().isEmpty ())
			IsSearching_ = false;

		QKeyEvent *ke = static_cast<QKeyEvent*> (e);
		if (!IsSearching_)
		{
			switch (ke->key ())
			{
			case Qt::Key_Space:
			case Qt::Key_Right:
			case Qt::Key_Left:
			case Qt::Key_Delete:
				qApp->sendEvent (View_, e);
				return true;
			default:
				;
			}
		}

		QList<int> intercepts
		{
			Qt::Key_Down, Qt::Key_Up,
			Qt::Key_PageDown, Qt::Key_PageUp,
			Qt::Key_Escape
		};
		if (InterceptEnter_)
			intercepts << Qt::Key_Enter << Qt::Key_Return;

		if (intercepts.contains (ke->key ()))
		{
			IsSearching_ = false;
			qApp->sendEvent (View_, e);
			return true;
		}

		IsSearching_ = true;
		return false;
	}
}
}
