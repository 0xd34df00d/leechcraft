/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QToolBar>
#include <QMap>
#include <QStringList>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/ijobholderrepresentationhandler.h>
#include "ui_summarywidget.h"
#include "core.h"

class QTimer;
class QComboBox;

namespace LC
{
namespace Summary
{
	class SearchWidget;
	class SummaryTagsFilter;

	class SummaryWidget : public QWidget
						, public ITabWidget
						, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::SummaryWidget Ui_;
		QTimer *FilterTimer_;

		SearchWidget * const SearchWidget_;

		std::unique_ptr<QToolBar> Toolbar_;
		static QObject *S_ParentMultiTabs_;

		SummaryTagsFilter *Sorter_;

		QHash<const QAbstractItemModel*, IJobHolderRepresentationHandler_ptr> SrcModel2Handler_;
		QSet<const QAbstractItemModel*> PreviouslySelectedModels_;
	public:
		explicit SummaryWidget (QWidget* = nullptr);
		~SummaryWidget () override;

		static void SetParentMultiTabs (QObject*);

		void Remove () override;
		QToolBar* GetToolBar () const override;
		QList<QAction*> GetTabBarContextMenuActions () const override;
		QObject* ParentMultiTabs () override;
		TabClassInfo GetTabClassInfo () const override;

		QByteArray GetTabRecoverData () const override;
		QString GetTabRecoverName () const override;
		QIcon GetTabRecoverIcon () const override;

		void RestoreState (const QByteArray&);

		void SetUpdatesEnabled (bool);

		Ui::SummaryWidget GetUi () const;
	private:
		SearchWidget* CreateSearchWidget ();
		void ReinitToolbar ();
		QList<QAction*> CreateProxyActions (const QList<QAction*>&, QObject*) const;
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
		void removeTab () override;
		void raiseTab () override;

		void tabRecoverDataChanged () override;
	};
}
}
