/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "summarywidget.h"
#include <algorithm>
#include <QGuiApplication>
#include <QMenu>
#include <QStyleHints>
#include <QToolBar>
#include <QWidgetAction>
#include <QLineEdit>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/progressdelegate.h>
#include <util/gui/viewselectiontracker.h>
#include <util/sll/qtutil.h>
#include "jobspresentationmodel.h"
#include "util.h"

namespace LC::Summary
{
	class SearchWidget : public QWidget
	{
		QHBoxLayout Layout_;
		QLineEdit Edit_;
	public:
		explicit SearchWidget (QWidget *parent)
		: QWidget { parent }
		{
			setLayout (&Layout_);

			Edit_.setPlaceholderText (SummaryWidget::tr ("Search..."));
			Edit_.setMaximumWidth (fontMetrics ().horizontalAdvance ('x') * 50);
			Edit_.setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
			Edit_.setClearButtonEnabled (true);
			Layout_.addStretch (1);
			Layout_.addWidget (&Edit_, 3);
		}

		QLineEdit& GetEdit ()
		{
			return Edit_;
		}
	};

	void SummaryWidget::SelectedModelsTracker::Refresh (const QModelIndexList& selected, const QModelIndex& current)
	{
		using RowSelection = IJobHolderRepresentationHandler::RowSelection;

		const auto& model2rows = Parent_.CollectModel2Rows (selected);
		const auto& curMapped = Parent_.MapToSource (current);
		for (const auto& [model, rows] : model2rows.asKeyValueRange ())
		{
			const auto& thisSelected = rows.contains (curMapped) ? curMapped : QModelIndex {};
			std::invoke (Callback_, *Parent_.SrcModel2Handler_.at (model), RowSelection { rows, thisSelected });
		}

		const QSet<const QAbstractItemModel*> curModels { model2rows.keyBegin (), model2rows.keyEnd () };
		for (const auto model : Models_ - curModels)
			std::invoke (Callback_, *Parent_.SrcModel2Handler_.at (model), RowSelection {});
		Models_ = curModels;
	}

	namespace
	{
		struct NullHandler : IJobHolderRepresentationHandler
		{
			QAbstractItemModel& GetRepresentation () override
			{
				throw std::runtime_error { "null model representation called" };
			}
		};

		std::optional<Util::ProgressDelegate::Progress> GetProgress (const QModelIndex& index)
		{
			const auto rowInfo = index.data (+JobHolderRole::RowInfo).value<RowInfo> ();
			const auto procInfo = std::get_if<ProcessInfo> (&rowInfo.Specific_);
			if (!procInfo)
				return {};

			const auto done = index.data (+JobHolderProcessRole::Done).value<qlonglong> ();
			const auto total = index.data (+JobHolderProcessRole::Total).value<qlonglong> ();

			auto scaledDone = done;
			auto scaledTotal = total;
			while (scaledTotal > std::numeric_limits<int>::max ())
			{
				scaledDone /= 10;
				scaledTotal /= 10;
			}

			return Util::ProgressDelegate::Progress
			{
				.Maximum_ = static_cast<int> (std::max (scaledTotal, 0LL)),
				.Progress_ = static_cast<int> (std::max (scaledDone, 0LL)),
				.Text_ = MakeProgressString (*procInfo, done, total, index),
			};
		}
	}

	SummaryWidget::SummaryWidget (QObject& parentPlugin)
	: Plugin_ { parentPlugin }
	, SearchWidget_ { new SearchWidget { this } }
	, Toolbar_ { new QToolBar }
	, MergeModel_ { { {}, {}, {} } }
	{
		SrcModel2Handler_ [nullptr] = std::make_unique<NullHandler> ();

		Toolbar_->setWindowTitle ("Summary");
		Toolbar_->addWidget (SearchWidget_);

		connect (&SearchWidget_->GetEdit (),
				&QLineEdit::textChanged,
				&FilterTimer_,
				qOverload<> (&QTimer::start));
		connect (&SearchWidget_->GetEdit (),
				&QLineEdit::returnPressed,
				this,
				[this]
				{
					FilterTimer_.stop ();
					SetFilterParams ();
				});

		Ui_.setupUi (this);
		Ui_.PluginsTasksTree_->setItemDelegateForColumn (JobsPresentationModel::Progress,
				new Util::ProgressDelegate { &GetProgress, this });

		FilterTimer_.setSingleShot (true);
		FilterTimer_.setInterval (QGuiApplication::styleHints ()->keyboardInputInterval ());
		FilterTimer_.callOnTimeout (this, &SummaryWidget::SetFilterParams);

		Ui_.ControlsDockWidget_->hide ();

		const IJobHolder::ViewCallbacks viewCallbacks
		{
			.SetSelection_ = std::bind_front (&SummaryWidget::SetSelection, this),
		};
		for (const auto plugin : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IJobHolder*> ())
			for (auto&& reprHandler : plugin->CreateRepresentationHandlers (viewCallbacks))
			{
				auto& model = reprHandler->GetRepresentation ();
				MergeModel_.AddModel (&model);
				SrcModel2Handler_ [&model] = std::move (reprHandler);
			}
		TagsFilterModel_.SetTagsRole (+CustomDataRoles::Tags);
		TagsFilterModel_.setSourceModel (&MergeModel_);
		PresentationModel_.setSourceModel (&TagsFilterModel_);
		Ui_.PluginsTasksTree_->setModel (&PresentationModel_);

		SelectionTracker_ = std::make_unique<Util::ViewSelectionTracker> (*Ui_.PluginsTasksTree_);
		connect (&*SelectionTracker_,
				&Util::ViewSelectionTracker::selectionChanging,
				this,
				[this]
				{
					EnsureControlsFor (Ui_.PluginsTasksTree_->currentIndex ());

					const auto sm = Ui_.PluginsTasksTree_->selectionModel ();
					Changing_.Refresh (sm->selectedRows (), sm->currentIndex ());
				});
		connect (&*SelectionTracker_,
				&Util::ViewSelectionTracker::selectionSettled,
				this,
				std::bind_front (&SelectedModelsTracker::Refresh, &Settled_));

		const auto connectAction = [this] (auto signal, auto method)
		{
			connect (Ui_.PluginsTasksTree_,
					signal,
					this,
					[this, method] (const QModelIndex& current)
					{
						const auto& thisMapped = MapToSource (current);
						std::invoke (method, GetHandler (thisMapped), thisMapped);
					});
		};

		connectAction (&QAbstractItemView::activated, &IJobHolderRepresentationHandler::HandleActivated);
		connectAction (&QAbstractItemView::clicked, &IJobHolderRepresentationHandler::HandleClicked);
		connectAction (&QAbstractItemView::doubleClicked, &IJobHolderRepresentationHandler::HandleDoubleClicked);
		connectAction (&QAbstractItemView::pressed, &IJobHolderRepresentationHandler::HandlePressed);

		connect (Ui_.PluginsTasksTree_,
				&QWidget::customContextMenuRequested,
				this,
				[this] (const QPoint& pos)
				{
					const auto& current = Ui_.PluginsTasksTree_->currentIndex ();
					if (const auto menu = GetHandler (MapToSource (current)).GetContextMenu ())
						menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
				});

		const auto itemsHeader = Ui_.PluginsTasksTree_->header ();
		const auto& fm = fontMetrics ();
		itemsHeader->resizeSection (0, fm.horizontalAdvance ("Average download job or torrent name is just like this."));
		itemsHeader->resizeSection (1, fm.horizontalAdvance ("Of the download."));
		itemsHeader->resizeSection (2, fm.horizontalAdvance ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));
	}

	SummaryWidget::~SummaryWidget ()
	{
		const auto widget = Ui_.ControlsDockWidget_->widget ();
		Ui_.ControlsDockWidget_->setWidget (nullptr);
		if (widget)
			widget->setParent (nullptr);
	}

	TabClassInfo SummaryWidget::GetStaticTabClassInfo ()
	{
		return
		{
			"Summary",
			tr ("Summary"),
			tr ("Summary of downloads and recent events."),
			GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon (),
			50,
			TFOpenableByRequest | TFByDefault | TFSuggestOpening
		};
	}

	void SummaryWidget::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* SummaryWidget::GetToolBar () const
	{
		return Toolbar_.get ();
	}

	QList<QAction*> SummaryWidget::GetTabBarContextMenuActions () const
	{
		return {};
	}

	QObject* SummaryWidget::ParentMultiTabs ()
	{
		return &Plugin_;
	}

	TabClassInfo SummaryWidget::GetTabClassInfo () const
	{
		return GetStaticTabClassInfo ();
	}

	QModelIndex SummaryWidget::MapToSource (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		return MergeModel_.mapToSource (TagsFilterModel_.mapToSource (PresentationModel_.mapToSource (index.siblingAtColumn (0))));
	}

	QModelIndex SummaryWidget::MapFromSource (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		return PresentationModel_.mapFromSource (TagsFilterModel_.mapFromSource (MergeModel_.mapFromSource (index)));
	}

	IJobHolderRepresentationHandler& SummaryWidget::GetHandler (const QModelIndex& index) const
	{
		const auto pos = SrcModel2Handler_.find (index.model ());
		if (pos == SrcModel2Handler_.end ())
			qFatal () << "no source model handler for" << index;
		return *pos->second;
	}

	void SummaryWidget::ClearToolbar ()
	{
		for (const auto action : Toolbar_->actions ())
			if (const auto wa = qobject_cast<QWidgetAction*> (action);
				!wa || wa->defaultWidget () != SearchWidget_)
				Toolbar_->removeAction (action);
	}

	std::optional<TabSaveInfo> SummaryWidget::GetTabSaveInfo () const
	{
		return { { .Name_ = GetTabClassInfo ().VisibleName_ } };
	}

	SummaryWidget::Model2Rows SummaryWidget::CollectModel2Rows (QModelIndexList indices) const
	{
		std::ranges::sort (indices, {}, &QModelIndex::row);

		Model2Rows newSelections;
		for (const auto& row : indices)
		{
			const auto& mapped = MapToSource (row);
			newSelections [mapped.model ()] << mapped;
		}
		return newSelections;
	}

	void SummaryWidget::SetSelection (const IJobHolderRepresentationHandler::RowSelection& selection)
	{
		QItemSelection visible;
		for (const auto& row : selection.Rows_)
			if (const auto& mapped = MapFromSource (row);
				mapped.isValid ())
				visible.select (mapped, mapped);

		auto curMapped = MapFromSource (selection.Current_);
		if (!curMapped.isValid () && !visible.isEmpty ())
			curMapped = visible.indexes ().front ();

		const auto sm = Ui_.PluginsTasksTree_->selectionModel ();
		sm->select (visible, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
		sm->setCurrentIndex (curMapped, QItemSelectionModel::NoUpdate);
	}

	void SummaryWidget::EnsureControlsFor (const QModelIndex& index)
	{
		const auto& srcIdx = MapToSource (index);
		const auto srcModel = srcIdx.model ();
		if (srcModel == CurrentModel_)
			return;

		CurrentModel_ = srcModel;

		auto& handler = GetHandler (srcIdx);

		ClearToolbar ();
		if (const auto toolbar = handler.GetControls ())
			Toolbar_->insertActions (Toolbar_->actions ().first (), toolbar->actions ());

		const auto info = handler.GetInfoWidget ();
		Ui_.ControlsDockWidget_->setWidget (info);
		Ui_.ControlsDockWidget_->setVisible (static_cast<bool> (info));
	}

	void SummaryWidget::SetFilterParams ()
	{
		TagsFilterModel_.SetFilterString (SearchWidget_->GetEdit ().text ());
	}
}
