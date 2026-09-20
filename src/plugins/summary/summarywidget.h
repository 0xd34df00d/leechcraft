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
#include <util/tags/tagsfiltermodel.h>
#include <interfaces/ijobholder.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_summarywidget.h"
#include "jobspresentationmodel.h"

namespace LC::Util
{
	class ViewSelectionTracker;
}

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
		Util::TagsFilterModel TagsFilterModel_;
		JobsPresentationModel PresentationModel_;

		std::unique_ptr<Util::ViewSelectionTracker> SelectionTracker_;

		std::unordered_map<const QAbstractItemModel*, IJobHolderRepresentationHandler_ptr> SrcModel2Handler_;

		struct SelectedModelsTracker
		{
			SummaryWidget& Parent_;
			void (IJobHolderRepresentationHandler::*Callback_) (const IJobHolderRepresentationHandler::RowSelection&);
			QSet<const QAbstractItemModel*> Models_ {};

			void Refresh (const QModelIndexList& selected, const QModelIndex& current);
		};

		SelectedModelsTracker Changing_ { *this, &IJobHolderRepresentationHandler::HandleSelectedRowsChanging };
		SelectedModelsTracker Settled_ { *this, &IJobHolderRepresentationHandler::HandleSelectedRowsSettled };

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

		std::optional<TabSaveInfo> GetTabSaveInfo () const override;
	private:
		QModelIndex MapToSource (const QModelIndex&) const;
		QModelIndex MapFromSource (const QModelIndex&) const;

		IJobHolderRepresentationHandler& GetHandler (const QModelIndex&) const;

		void ClearToolbar ();
		void SetFilterParams ();

		using Model2Rows = QHash<const QAbstractItemModel*, QModelIndexList>;
		Model2Rows CollectModel2Rows (QModelIndexList) const;

		void SetSelection (const IJobHolderRepresentationHandler::RowSelection&);
		void EnsureControlsFor (const QModelIndex&);
	signals:
		void removeTab () override;
		void raiseTab () override;

		void tabRecoverDataChanged () override;
	};
}
