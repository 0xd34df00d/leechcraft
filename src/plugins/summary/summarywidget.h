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

#include <QWidget>
#include <QToolBar>
#include <QMap>
#include <QStringList>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_summarywidget.h"
#include "core.h"

class QTimer;
class QComboBox;

namespace LeechCraft
{
namespace Summary
{
	class SearchWidget;

	class SummaryWidget : public QWidget
						, public ITabWidget
						, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::SummaryWidget Ui_;
		QTimer *FilterTimer_;

		SearchWidget *SearchWidget_;

		std::unique_ptr<QToolBar> Toolbar_;
		static QObject *S_ParentMultiTabs_;

		QSortFilterProxyModel *Sorter_;
	public:
		SummaryWidget (QWidget* = 0);
		virtual ~SummaryWidget ();

		static void SetParentMultiTabs (QObject*);

		void Remove ();
		QToolBar* GetToolBar () const;
		QList<QAction*> GetTabBarContextMenuActions () const;
		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;

		QByteArray GetTabRecoverData () const;
		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;

		void RestoreState (const QByteArray&);

		void SetUpdatesEnabled (bool);

		Ui::SummaryWidget GetUi () const;
	private:
		void ReconnectModelSpecific ();
		void ConnectObject (QObject*);
		SearchWidget* CreateSearchWidget ();
		void ReinitToolbar ();
		QList<QAction*> CreateProxyActions (const QList<QAction*>&) const;
	private slots:
		void handleActionTriggered (QAction*);
		void checkDataChanged (const QModelIndex&, const QModelIndex&);
		void handleReset ();
		void checkRowsToBeRemoved (const QModelIndex&, int, int);
		void updatePanes (const QModelIndex&, const QModelIndex&);
		void filterParametersChanged ();
		void filterReturnPressed ();
		void feedFilterParameters ();
		void on_PluginsTasksTree__customContextMenuRequested (const QPoint&);
		void syncSelection (const QModelIndex&);
	signals:
		void changeTabName (const QString&);
		void raiseTab (QWidget*);
		void needToClose ();

		void tabRecoverDataChanged ();
	};
}
}
