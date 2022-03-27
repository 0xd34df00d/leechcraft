/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPair>

namespace LC
{
namespace TabSessManager
{
	struct RecInfo;
	class TabsPropsManager;

	class SessionsManager : public QObject
	{
		Q_OBJECT

		TabsPropsManager * const TabsPropsMgr_;

		bool IsScheduled_ = false;
		bool IsRecovering_ = true;

		QList<QList<QObject*>> Tabs_;
	public:
		SessionsManager (TabsPropsManager*,  QObject* = nullptr);

		QStringList GetCustomSessions () const;

		bool HasTab (QObject*);

		QHash<QObject*, QList<RecInfo>> GetTabsInSession (const QString&) const;

		void OpenTabs (const QHash<QObject*, QList<RecInfo>>&);
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		QByteArray GetCurrentSession () const;
	public slots:
		void recover ();
		void handleTabRecoverDataChanged ();

		void saveDefaultSession ();
		void saveCustomSession ();

		void loadCustomSession (const QString&);
		void addCustomSession (const QString&);
		void deleteCustomSession (const QString&);

		void handleRemoveTab (QWidget*);
	private slots:
		void handleNewTab (const QString&, QWidget*);
		void handleTabMoved (int, int);

		void handleWindow (int);
		void handleWindowRemoved (int);
	signals:
		void gotCustomSession (const QString&);
	};
}
}
