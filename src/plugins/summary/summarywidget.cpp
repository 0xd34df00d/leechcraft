/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "summarywidget.h"
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
#include <util/sll/qtutil.h>
#include "summarytagsfilter.h"
#include "modeldelegate.h"

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

	namespace
	{
		struct GuardHandler : IJobHolderRepresentationHandler
		{
			QAbstractItemModel& GetRepresentation () override
			{
				throw std::runtime_error { "null model representation called" };
			}
		};
	}

	SummaryWidget::SummaryWidget (QObject& plugin)
	: Plugin_ { plugin }
	, SearchWidget_ { new SearchWidget { this } }
	, Toolbar_ { new QToolBar }
	, MergeModel_ { { {}, {}, {} } }
	{
		SrcModel2Handler_ [nullptr] = std::make_unique<GuardHandler> ();

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
		Ui_.PluginsTasksTree_->setItemDelegate (new ModelDelegate (this));

		FilterTimer_.setSingleShot (true);
		FilterTimer_.setInterval (QGuiApplication::styleHints ()->keyboardInputInterval ());
		FilterTimer_.callOnTimeout (this, &SummaryWidget::SetFilterParams);

		Ui_.ControlsDockWidget_->hide ();

		for (const auto plugin : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IJobHolder*> ())
		{
			auto reprHandler = plugin->CreateRepresentationHandler ();
			auto& model = reprHandler->GetRepresentation ();
			MergeModel_.AddModel (&model);
			SrcModel2Handler_ [&model] = std::move (reprHandler);
		}
		Filter_.setSourceModel (&MergeModel_);
		Ui_.PluginsTasksTree_->setModel (&Filter_);

		connect (Ui_.PluginsTasksTree_->selectionModel (),
				&QItemSelectionModel::currentRowChanged,
				this,
				&SummaryWidget::SetCurrentRow);
		connect (Ui_.PluginsTasksTree_->selectionModel (),
				&QItemSelectionModel::currentRowChanged,
				this,
				&SummaryWidget::EnsureCurrentRowSelected);

		auto connectViewSignal = [this] (auto emitter, auto signal, auto method)
		{
			connect (emitter,
					signal,
					this,
					[this, method] (const QModelIndex& current)
					{
						const auto& thisMapped = MapToSourceRecursively (current);
						std::invoke (method, GetHandler (thisMapped), thisMapped);
					});
		};

		const auto connectChange = std::bind_front (connectViewSignal, Ui_.PluginsTasksTree_->selectionModel ());
		connectChange (&QItemSelectionModel::currentChanged,
				&IJobHolderRepresentationHandler::HandleCurrentChanged);
		connectChange (&QItemSelectionModel::currentRowChanged,
				&IJobHolderRepresentationHandler::HandleCurrentRowChanged);
		connectChange (&QItemSelectionModel::currentColumnChanged,
				&IJobHolderRepresentationHandler::HandleCurrentColumnChanged);

		const auto connectAction = std::bind_front (connectViewSignal, Ui_.PluginsTasksTree_);
		connectAction (&QAbstractItemView::activated, &IJobHolderRepresentationHandler::HandleActivated);
		connectAction (&QAbstractItemView::clicked, &IJobHolderRepresentationHandler::HandleClicked);
		connectAction (&QAbstractItemView::doubleClicked, &IJobHolderRepresentationHandler::HandleDoubleClicked);
		connectAction (&QAbstractItemView::entered, &IJobHolderRepresentationHandler::HandleEntered);
		connectAction (&QAbstractItemView::pressed, &IJobHolderRepresentationHandler::HandlePressed);

		connect (Ui_.PluginsTasksTree_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				[this]
				{
					QHash<const QAbstractItemModel*, QModelIndexList> newSelections;
					for (const auto& row : Ui_.PluginsTasksTree_->selectionModel ()->selectedRows ())
					{
						const auto& mapped = MapToSourceRecursively (row);
						newSelections [mapped.model ()] << mapped;
					}

					for (const auto& [model, rows] : Util::Stlize (newSelections))
						SrcModel2Handler_.at (model)->HandleSelectedRowsChanged (rows);

					QSet<const QAbstractItemModel*> curModels { newSelections.keyBegin (), newSelections.keyEnd () };
					for (const auto model : PreviouslySelectedModels_ - curModels)
						SrcModel2Handler_.at (model)->HandleSelectedRowsChanged ({});
					PreviouslySelectedModels_ = curModels;
				});

		connect (Ui_.PluginsTasksTree_,
				&QWidget::customContextMenuRequested,
				this,
				[this] (const QPoint& pos)
				{
					const auto& current = Ui_.PluginsTasksTree_->currentIndex ();
					if (const auto menu = current.data (+CustomDataRoles::ContextMenu).value<QMenu*> ())
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
		return QList<QAction*> ();
	}

	QObject* SummaryWidget::ParentMultiTabs ()
	{
		return &Plugin_;
	}

	TabClassInfo SummaryWidget::GetTabClassInfo () const
	{
		return GetStaticTabClassInfo ();
	}

	QModelIndex SummaryWidget::MapToSourceRecursively (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		return MergeModel_.mapToSource (Filter_.mapToSource (index));
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

	QByteArray SummaryWidget::GetTabRecoverData () const
	{
		return {};
	}

	QString SummaryWidget::GetTabRecoverName () const
	{
		return GetTabClassInfo ().VisibleName_;
	}

	QIcon SummaryWidget::GetTabRecoverIcon () const
	{
		return GetTabClassInfo ().Icon_;
	}

	void SummaryWidget::SetCurrentRow (const QModelIndex& index)
	{
		const auto& srcIdx = MapToSourceRecursively (index);
		const auto srcModel = srcIdx.model ();
		if (srcModel == CurrentModel_)
			return;

		const auto& handler = SrcModel2Handler_.at (CurrentModel_);
		handler->HandleCurrentChanged ({});
		handler->HandleCurrentRowChanged ({});
		handler->HandleCurrentColumnChanged ({});

		CurrentModel_ = srcModel;

		ClearToolbar ();
		if (const auto toolbar = srcIdx.data (+CustomDataRoles::Controls).value<QToolBar*> ())
			Toolbar_->insertActions (Toolbar_->actions ().first (), toolbar->actions ());

		const auto info = GetHandler (srcIdx).GetInfoWidget ();
		Ui_.ControlsDockWidget_->setWidget (info);
		Ui_.ControlsDockWidget_->setVisible (static_cast<bool> (info));
	}

	void SummaryWidget::SetFilterParams ()
	{
		Filter_.SetFilterString (SearchWidget_->GetEdit ().text ());
	}

	void SummaryWidget::EnsureCurrentRowSelected (const QModelIndex&)
	{
		const auto selm = Ui_.PluginsTasksTree_->selectionModel ();
		if (const auto& cur = selm->currentIndex ();
			cur.isValid () && !selm->rowIntersectsSelection (cur.row ()))
			selm->select (cur, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}
}
