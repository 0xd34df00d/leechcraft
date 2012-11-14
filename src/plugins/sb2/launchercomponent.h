/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

namespace LeechCraft
{
struct TabClassInfo;
struct QuarkComponent;

namespace SB2
{
	class TabClassImageProvider;
	class TabListView;

	class LauncherComponent : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QStandardItemModel *Model_;
		QuarkComponent Component_;

		TabClassImageProvider *ImageProv_;
		QHash<QByteArray, IHaveTabs*> TC2Obj_;
		QHash<QByteArray, QList<QStandardItem*>> TC2Items_;
		QHash<QByteArray, QList<QWidget*>> TC2Widgets_;

		QPointer<TabListView> CurrentTabList_;

		QSet<QByteArray> HiddenTCs_;
	public:
		LauncherComponent (ICoreProxy_ptr, QObject* = 0);

		QuarkComponent GetComponent () const;
	private:
		void SaveHiddenTCs () const;
		void LoadHiddenTCs ();

		QStandardItem* TryAddTC (const TabClassInfo&);
		QStandardItem* CreateItem (const TabClassInfo&);
	public slots:
		void handlePluginsAvailable ();

		void tabOpenRequested (const QByteArray&);
		void tabClassHideRequested (const QByteArray&);
		void tabClassUnhideRequested (const QByteArray&);
		void tabListRequested (const QByteArray&, int, int);
		void tabListUnhovered (const QByteArray&);
	private slots:
		void handleNewTab (const QString&, QWidget*);
		void handleRemoveTab (QWidget*);
		void handleCurrentTabChanged (int);
	};
}
}
