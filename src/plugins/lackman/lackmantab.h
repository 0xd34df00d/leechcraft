/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_lackmantab.h"

class QStringListModel;

namespace LC
{
namespace Util
{
	class ShortcutManager;
}
namespace LackMan
{
	class TypeFilterProxyModel;
	class StringFilterModel;

	class LackManTab : public QWidget
					 , public ITabWidget
					 , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::LackManTab Ui_;

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;

		Util::ShortcutManager * const ShortcutMgr_;

		QStringListModel * const TagsModel_;

		StringFilterModel * const FilterString_;
		TypeFilterProxyModel * const TypeFilter_;

		QAction *UpdateAll_;
		QAction *UpgradeAll_;
		QAction *Apply_;
		QAction *Cancel_;
		QToolBar *Toolbar_;
	public:
		LackManTab (Util::ShortcutManager*, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;

		void SetFilterTags (const QStringList&);
		void SetFilterString (const QString&);
	private:
		void BuildPackageTreeShortcuts ();
		void BuildActions ();
	private slots:
		void navigateUp ();
		void navigateDown ();
		void toggleSelected ();

		void handlePackageSelected (const QModelIndex&);
		void handleFetchListUpdated (const QList<int>&);
		void handleTagsUpdated (const QStringList&);

		void toggleInstall ();
		void toggleUpgrade ();
		void selectAllForInstall ();
		void selectNoneForInstall ();

		void on_PackagesTree__customContextMenuRequested (const QPoint&);
		void on_PackageStatus__currentIndexChanged (int);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void tabRecoverDataChanged ();
	};
}
}
