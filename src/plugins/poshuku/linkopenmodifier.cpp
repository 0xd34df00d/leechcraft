/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkopenmodifier.h"
#include <QMouseEvent>
#include <QChildEvent>
#include <QWidget>
#include <QtDebug>
#include "xmlsettingsmanager.h"

namespace LC::Poshuku
{
	void LinkOpenModifier::InstallOn (QWidget *view)
	{
		view->installEventFilter (this);
	}

	auto LinkOpenModifier::GetOpenBehaviourSuggestion () const -> OpenBehaviourSuggestion
	{
		const auto newTabRequested = MouseButtons_ == Qt::MiddleButton ||
				Modifiers_ & Qt::ControlModifier;
		if (!newTabRequested)
			return {};

		auto background = XmlSettingsManager::Instance ().property ("BackgroundNewTabs").toBool ();
		if (Modifiers_ & Qt::ShiftModifier)
			background = !background;
		return { true, background };
	}

	void LinkOpenModifier::ResetSuggestionState ()
	{
		MouseButtons_ = {};
		Modifiers_ = {};
	}

	bool LinkOpenModifier::eventFilter (QObject*, QEvent *e)
	{
		switch (e->type ())
		{
		case QEvent::MouseButtonPress:
		{
			const auto me = static_cast<QMouseEvent*> (e);
			MouseButtons_ = me->buttons ();
			Modifiers_ = me->modifiers ();
			break;
		}
		case QEvent::ChildAdded:
			static_cast<QChildEvent*> (e)->child ()->installEventFilter (this);
			break;
		case QEvent::ChildRemoved:
			static_cast<QChildEvent*> (e)->child ()->removeEventFilter (this);
			break;
		default:
			break;
		}

		return false;
	}
}
