/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_SUMMARY_SUMMARYWIDGET_H
#define PLUGINS_SUMMARY_SUMMARYWIDGET_H
#include <QWidget>
#include <QStringList>
#include <interfaces/imultitabs.h>
#include "ui_summarywidget.h"
#include "core.h"

class QTimer;
class QComboBox;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			class SearchWidget;

			class SummaryWidget : public QWidget
								, public IMultiTabsWidget 
			{
				Q_OBJECT
				Q_INTERFACES (IMultiTabsWidget)

				Ui::SummaryWidget Ui_;
				QTimer *FilterTimer_;
				QList<QComboBox*> AdditionalBoxes_;
				QToolBar *Toolbar_;
				QAction *ActionSearch_;
				SearchWidget *SearchWidget_;
				static QObject *S_ParentMultiTabs_;
			public:
				SummaryWidget (QWidget* = 0);
				virtual ~SummaryWidget ();
				static void SetParentMultiTabs (QObject*);

				void Remove ();
				QToolBar* GetToolBar () const;
				void NewTabRequested ();
				QList<QAction*> GetTabBarContextMenuActions () const;
				QObject* ParentMultiTabs () const;

				void SmartDeselect (SummaryWidget*);
				Ui::SummaryWidget GetUi () const;
				void SetQuery (QStringList);
			private:
				QStringList GetUniqueCategories () const;
				void FillCombobox (QComboBox*);
				QString GetQuery () const;
				Query2 GetQuery2 () const;
				void ReinitToolbar ();
			private slots:
				void updatePanes (const QModelIndex&, const QModelIndex&);
				void filterParametersChanged ();
				void filterReturnPressed ();
				void feedFilterParameters ();
				void on_PluginsTasksTree__customContextMenuRequested (const QPoint&);
				void addCategoryBox ();
				void handleCategoriesChanged (const QStringList&, const QStringList&);
				void removeCategoryBox ();
				void syncSelection (const QModelIndex&);
			signals:
				void changeTabName (const QString&);
				void filterUpdated ();
				void queryUpdated (const QString&);
				void queryUpdated (const LeechCraft::Plugins::Summary::Query2&);
				void needToClose ();
				void newTabRequested ();
			};
		};
	};
};

#endif

