/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include "eventdata.h"

namespace LC::AdvancedNotifications
{
	class EventProxyObject : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (int count READ count NOTIFY countChanged)
		Q_PROPERTY (QUrl image READ image NOTIFY imageChanged)
		Q_PROPERTY (QString extendedText READ extendedText NOTIFY extendedTextChanged)
		Q_PROPERTY (QVariant eventActionsModel READ eventActionsModel NOTIFY eventActionsModelChanged)

		EventData E_;
		QUrl CachedImage_;
		QVariant ActionsModel_;
	public:
		explicit EventProxyObject (const EventData&, QObject* = nullptr);

		int count () const;
		QUrl image () const;
		QString extendedText () const;

		QVariant eventActionsModel () const;
	signals:
		void countChanged ();
		void imageChanged ();
		void extendedTextChanged ();

		void eventActionsModelChanged ();

		void dismissEvent ();

		void actionTriggered (const QString&, int);
		void dismissEventRequested (const QString&);
	};
}
