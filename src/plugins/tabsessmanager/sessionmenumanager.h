/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QCoreApplication>
#include <QObject>
#include <QHash>

class QMenu;
class QAction;

namespace LC::TabSessManager
{
	class SessionsManager;

	class SessionMenuManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::TabSessManager::SessionMenuManager)

		SessionsManager * const SessMgr_;

		QMenu * const SessMgrMenu_;

		QHash<QString, std::shared_ptr<QMenu>> Session2Menu_;
	public:
		SessionMenuManager (SessionsManager*, QObject* = nullptr);

		QAction* GetSessionsAction () const;
	private:
		void DeleteSession (const QString&);
	public slots:
		void addCustomSession (const QString&);
	signals:
		void loadRequested (const QString&);
		void addRequested (const QString&);
		void deleteRequested (const QString&);

		void saveCustomSessionRequested ();
	};
}
