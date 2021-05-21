/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::AdvancedNotifications
{
	class ActionsProxyObject : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (QString actionText READ actionText NOTIFY actionTextChanged)

		QString ActionText_;
	public:
		explicit ActionsProxyObject (const QString&, QObject* = nullptr);

		QString actionText () const;
	signals:
		void actionTextChanged ();

		void actionSelected ();
	};
}
