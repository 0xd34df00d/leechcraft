/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QSet>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;
class QStandardItem;
class IHaveTabs;

namespace LC
{
struct TabClassInfo;
class QuarkComponent;

namespace SB2
{
	class ViewManager;
	class TabClassImageProvider;
	class TabListView;

	class LauncherComponent : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		ICoreTabWidget *ICTW_;
		QStandardItemModel *Model_;
		QuarkComponent_ptr Component_;

		ViewManager *View_;

		TabClassImageProvider *ImageProv_;
		QHash<QByteArray, IHaveTabs*> TC2Obj_;
		QHash<QByteArray, QList<QStandardItem*>> TC2Items_;
		QHash<QByteArray, QList<QWidget*>> TC2Widgets_;

		QPointer<TabListView> CurrentTabList_;

		QSet<QByteArray> HiddenTCs_;

		bool FirstRun_;
	public:
		LauncherComponent (ICoreTabWidget*, ICoreProxy_ptr, ViewManager*, QObject* = 0);

		QuarkComponent_ptr GetComponent () const;
	private:
		void SaveHiddenTCs () const;
		void LoadHiddenTCs ();

		QStandardItem* TryAddTC (const TabClassInfo&);
		QStandardItem* CreateItem (const TabClassInfo&);

		QPair<TabClassInfo, IHaveTabs*> FindTC (const QByteArray&) const;
	public slots:
		void handlePluginsAvailable ();

		void tabOpenRequested (const QByteArray&);
		void tabClassHideRequested (const QByteArray&);
		void tabClassUnhideRequested (const QByteArray&);
		void tabUnhideListRequested (int, int);
		void tabListRequested (const QByteArray&, int, int);
		void tabListUnhovered (const QByteArray&);
	private slots:
		void handleNewTab (const QString&, QWidget*);
		void handleRemoveTab (QWidget*);
		void handleCurrentTabChanged (int);
	};
}
}
