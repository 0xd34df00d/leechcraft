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

namespace LC::TabSessManager
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
		explicit SessionsManager (TabsPropsManager*,  QObject* = nullptr);

		QStringList GetCustomSessions () const;

		bool HasTab (QObject*);

		QHash<QObject*, QList<RecInfo>> GetTabsInSession (const QString&) const;

		void OpenTabs (const QHash<QObject*, QList<RecInfo>>&);

		void Recover ();

		void SaveCustomSession ();
		void LoadCustomSession (const QString&);
		void AddCustomSession (const QString&);
		void DeleteCustomSession (const QString&);
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	private:
		QByteArray GetCurrentSession () const;
	public slots:
		void handleTabRecoverDataChanged ();

		void handleRemoveTab (QWidget*);
	private slots:
		void handleNewTab (int, QWidget*);
		void handleTabMoved (int, int);

		void handleWindow (int);
		void handleWindowRemoved (int);
	signals:
		void gotCustomSession (const QString&);
	};
}
