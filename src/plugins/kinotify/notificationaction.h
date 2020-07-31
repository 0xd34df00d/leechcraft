/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Kinotify
{
	class NotificationAction : public QObject
	{
		Q_OBJECT

		QObject *ActionObject_;
	public:
		NotificationAction (QObject *);
		void SetActionObject (QObject*);
	public slots:
		void sendActionOnClick (const QString&);
	signals:
		void actionPressed ();
	};
}
}
