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
#include "jobspresentationmodel.h"

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
		JobsPresentationModel Filter_;

		std::unordered_map<const QAbstractItemModel*, IJobHolderRepresentationHandler_ptr> SrcModel2Handler_;
		QSet<const QAbstractItemModel*> PreviouslySelectedModels_;

		const QAbstractItemModel *CurrentModel_ = nullptr;
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

		void ClearToolbar ();

		void SetFilterParams ();

		void SetCurrentRow (const QModelIndex&);
		void EnsureCurrentRowSelected ();
	signals:
		void removeTab () override;
		void raiseTab () override;

		void tabRecoverDataChanged () override;
	};
}
