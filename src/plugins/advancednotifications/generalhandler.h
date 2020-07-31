/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <QIcon>
#include <interfaces/iinfo.h>
#include <interfaces/iactionsexporter.h>
#include "concretehandlerbase.h"

namespace LC
{
namespace AdvancedNotifications
{
	class RulesManager;
	class AudioThemeManager;
	class UnhandledNotificationsKeeper;

	class GeneralHandler : public QObject
	{
		Q_OBJECT

		RulesManager * const RulesManager_;
		UnhandledNotificationsKeeper * const UnhandledKeeper_;
		ICoreProxy_ptr Proxy_;

		QList<INotificationHandler_ptr> Handlers_;

		QMap<QString, QString> Cat2IconName_;
	public:
		GeneralHandler (RulesManager*, const AudioThemeManager*,
				UnhandledNotificationsKeeper*, const ICoreProxy_ptr&);

		void RegisterHandler (const INotificationHandler_ptr&);

		void Handle (const Entity&);

		ICoreProxy_ptr GetProxy () const;
		QIcon GetIconForCategory (const QString&) const;
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace);
	};
}
}
