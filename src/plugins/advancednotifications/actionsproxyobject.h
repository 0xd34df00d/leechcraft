/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_QML_ACTIONSPROXYOBJECT_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_QML_ACTIONSPROXYOBJECT_H
#include <QObject>

namespace LC
{
namespace AdvancedNotifications
{
	class ActionsProxyObject : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (QString actionText READ actionText NOTIFY actionTextChanged)

		QString ActionText_;
	public:
		ActionsProxyObject (const QString&, QObject* = 0);

		QString actionText () const;
	signals:
		void actionTextChanged ();

		void actionSelected ();
	};
}
}

#endif
