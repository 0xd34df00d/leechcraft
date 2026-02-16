/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <unordered_map>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <util/models/mergemodel.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_summarywidget.h"
#include "summarytagsfilter.h"

namespace LC::Summary
{
	class SearchWidget;

	class SummaryWidget
		: public QWidget
		, public ITabWidget
		, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::SummaryWidget Ui_;

		QObject& Plugin_;

		QTimer FilterTimer_;

		SearchWidget * const SearchWidget_;

		std::unique_ptr<QToolBar> Toolbar_;

		Util::MergeModel MergeModel_;
		SummaryTagsFilter Filter_;

		std::unordered_map<const QAbstractItemModel*, IJobHolderRepresentationHandler_ptr> SrcModel2Handler_;
		QSet<const QAbstractItemModel*> PreviouslySelectedModels_;
	public:
		explicit SummaryWidget (QObject& parentPlugin);
		~SummaryWidget () override;

		static TabClassInfo GetStaticTabClassInfo ();

		void Remove () override;
		QToolBar* GetToolBar () const override;
		QList<QAction*> GetTabBarContextMenuActions () const override;
		QObject* ParentMultiTabs () override;
		TabClassInfo GetTabClassInfo () const override;

		QByteArray GetTabRecoverData () const override;
		QString GetTabRecoverName () const override;
		QIcon GetTabRecoverIcon () const override;
	private:
		QModelIndex MapToSourceRecursively (const QModelIndex&) const;
		IJobHolderRepresentationHandler& GetHandler (const QModelIndex&) const;

		void ReinitToolbar ();

		void SetFilterParams ();
	private slots:
		void handleReset ();
		void checkRowsToBeRemoved (const QModelIndex&, int, int);
		void updatePanes (const QModelIndex&, const QModelIndex&);
		void on_PluginsTasksTree__customContextMenuRequested (const QPoint&);
		void syncSelection (const QModelIndex&);
	signals:
		void removeTab () override;
		void raiseTab () override;

		void tabRecoverDataChanged () override;
	};
}
