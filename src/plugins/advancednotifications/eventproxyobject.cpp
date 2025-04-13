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

namespace LC::AdvancedNotifications
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
			auto proxy = new ActionsProxyObject (action, this);
			connect (proxy,
					&ActionsProxyObject::actionSelected,
					[this, idx = i++]
					{
						emit actionTriggered ({ E_.SenderId_, E_.EventId_ }, idx);
					});
			model << proxy;
		}

		connect (this,
				&EventProxyObject::dismissEvent,
				this,
				[this] { emit dismissEventRequested ({ E_.SenderId_, E_.EventId_ }); },
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
}
