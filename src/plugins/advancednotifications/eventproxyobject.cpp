/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eventproxyobject.h"
#include <QVariant>
#include <util/util.h>
#include "actionsproxyobject.h"

Q_DECLARE_METATYPE (QList<QObject*>)

namespace LC
{
namespace AdvancedNotifications
{
	EventProxyObject::EventProxyObject (const EventData& ed, QObject *parent)
	: QObject (parent)
	, E_ (ed)
	{
		CachedImage_ = QUrl (Util::GetAsBase64Src (E_.Pixmap_.scaled (32, 32).toImage ()));

		QList<QObject*> model;
		int i = 0;
		for (const auto& action : ed.Actions_)
		{
			QObject *proxy = new ActionsProxyObject (action, this);
			proxy->setProperty ("ActionIndex", i++);
			connect (proxy,
					SIGNAL (actionSelected ()),
					this,
					SLOT (handleActionSelected ()));
			model << proxy;
		}

		connect (this,
				SIGNAL (dismissEvent ()),
				this,
				SLOT (handleDismissEvent ()),
				Qt::QueuedConnection);

		ActionsModel_ = QVariant::fromValue<QList<QObject*>> (model);
	}

	int EventProxyObject::count () const
	{
		return E_.Count_;
	}

	QUrl EventProxyObject::image () const
	{
		return CachedImage_;
	}

	QString EventProxyObject::extendedText () const
	{
		return E_.FullText_.isEmpty () ?
				E_.ExtendedText_ :
				E_.FullText_;
	}

	QVariant EventProxyObject::eventActionsModel () const
	{
		return ActionsModel_;
	}

	void EventProxyObject::handleActionSelected ()
	{
		const int idx = sender ()->property ("ActionIndex").toInt ();
		emit actionTriggered (E_.EventID_, idx);
	}

	void EventProxyObject::handleDismissEvent ()
	{
		emit dismissEventRequested (E_.EventID_);
	}
}
}
