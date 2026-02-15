/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "summarywidget.h"
#include <QMenu>
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
		connect (Toolbar_.get (),
				SIGNAL (actionTriggered (QAction*)),
				this,
				SLOT (handleActionTriggered (QAction*)));

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
		FilterTimer_.setInterval (800);
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

		connect (&Filter_,
				SIGNAL (modelAboutToBeReset ()),
				this,
				SLOT (handleReset ()));
		connect (&Filter_,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
				this,
				SLOT (checkRowsToBeRemoved (QModelIndex, int, int)));
		connect (Ui_.PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (updatePanes (QModelIndex, QModelIndex)));
		connect (Ui_.PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (syncSelection (QModelIndex)),
				Qt::QueuedConnection);

		const auto itemsHeader = Ui_.PluginsTasksTree_->header ();
		const auto& fm = fontMetrics ();
		itemsHeader->resizeSection (0, fm.horizontalAdvance ("Average download job or torrent name is just like this."));
		itemsHeader->resizeSection (1, fm.horizontalAdvance ("Of the download."));
		itemsHeader->resizeSection (2, fm.horizontalAdvance ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

		auto connectChange = [this] (auto signal, auto method)
		{
			connect (Ui_.PluginsTasksTree_->selectionModel (),
					signal,
					this,
					[this, method] (const QModelIndex& current, const QModelIndex& previous)
					{
						const auto& prevMapped = MapToSourceRecursively (previous);
						const auto& thisMapped = MapToSourceRecursively (current);

						if (prevMapped.isValid () && prevMapped.model () != thisMapped.model ())
							std::invoke (method, GetHandler (prevMapped), QModelIndex {});

						if (thisMapped.isValid ())
							std::invoke (method, GetHandler (thisMapped), thisMapped);
					});
		};
		connectChange (&QItemSelectionModel::currentChanged,
				&IJobHolderRepresentationHandler::HandleCurrentChanged);
		connectChange (&QItemSelectionModel::currentRowChanged,
				&IJobHolderRepresentationHandler::HandleCurrentRowChanged);
		connectChange (&QItemSelectionModel::currentColumnChanged,
				&IJobHolderRepresentationHandler::HandleCurrentColumnChanged);

		auto connectAction = [this] (auto signal, auto method)
		{
			connect (Ui_.PluginsTasksTree_,
					signal,
					this,
					[this, method] (const QModelIndex& index)
					{
						const auto& mapped = MapToSourceRecursively (index);
						if (mapped.isValid ())
							std::invoke (method, GetHandler (mapped), mapped);
					});
		};
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

	void SummaryWidget::ReinitToolbar ()
	{
		for (const auto action : Toolbar_->actions ())
			if (auto wa = qobject_cast<QWidgetAction*> (action);
				!wa || wa->defaultWidget () != SearchWidget_)
			{
				Toolbar_->removeAction (action);
				delete action;
			}
	}

	QList<QAction*> SummaryWidget::CreateProxyActions (const QList<QAction*>& actions, QObject *parent) const
	{
		QList<QAction*> proxies;

		for (const auto action : actions)
		{
			if (qobject_cast<QWidgetAction*> (action))
			{
				proxies << action;
				continue;
			}

			QAction *pa = new QAction (action->icon (), action->text (), parent);
			if (action->isSeparator ())
				pa->setSeparator (true);
			else
			{
				pa->setCheckable (action->isCheckable ());
				pa->setChecked (action->isChecked ());
				pa->setShortcuts (action->shortcuts ());
				pa->setStatusTip (action->statusTip ());
				pa->setToolTip (action->toolTip ());
				pa->setWhatsThis (action->whatsThis ());
				pa->setData (QVariant::fromValue<QObject*> (action));

				connect (pa,
						SIGNAL (hovered ()),
						action,
						SIGNAL (hovered ()));
				connect (pa,
						SIGNAL (toggled (bool)),
						action,
						SIGNAL (toggled (bool)));
			}
			proxies << pa;
		}

		return proxies;
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

	void SummaryWidget::handleActionTriggered (QAction *proxyAction)
	{
		QAction *action = qobject_cast<QAction*> (proxyAction->
				data ().value<QObject*> ());
		QItemSelectionModel *selModel =
				Ui_.PluginsTasksTree_->selectionModel ();
		QModelIndexList indexes = selModel->selectedRows ();
		action->setProperty ("SelectedRows",
				QVariant::fromValue<QList<QModelIndex>> (indexes));
		action->setProperty ("ItemSelectionModel",
				QVariant::fromValue<QObject*> (selModel));

		action->activate (QAction::Trigger);
	}

	void SummaryWidget::handleReset ()
	{
		Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void SummaryWidget::checkRowsToBeRemoved (const QModelIndex&, int begin, int end)
	{
		const auto& cur = Ui_.PluginsTasksTree_->selectionModel ()->currentIndex ();
		if (begin <= cur.row () && end >= cur.row ())
			Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void SummaryWidget::updatePanes (const QModelIndex& newIndex, const QModelIndex& oldIndex)
	{
		const auto& newSrcIdx = MapToSourceRecursively (newIndex);
		const auto& oldSrcIdx = MapToSourceRecursively (oldIndex);
		if (const auto toolbar = newSrcIdx.data (+CustomDataRoles::Controls).value<QToolBar*> ();
			toolbar != oldSrcIdx.data (+CustomDataRoles::Controls).value<QToolBar*> ())
		{
			ReinitToolbar ();
			if (toolbar)
			{
				for (const auto action : toolbar->actions ())
				{
					const auto& ai = action->property ("ActionIcon").toString ();
					if (!ai.isEmpty () && action->icon ().isNull ())
						action->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetIcon (ai));
				}

				const auto& proxies = CreateProxyActions (toolbar->actions (), Toolbar_.get ());
				Toolbar_->insertActions (Toolbar_->actions ().first (), proxies);
			}
		}

		if (const auto info = GetHandler (newSrcIdx).GetInfoWidget ();
			info != GetHandler (oldSrcIdx).GetInfoWidget ())
		{
			Ui_.ControlsDockWidget_->setWidget (info);
			Ui_.ControlsDockWidget_->setVisible (static_cast<bool> (info));
			if (info)
				GetProxyHolder ()->GetIconThemeManager ()->UpdateIconset (info->findChildren<QAction*> ());
		}
	}

	void SummaryWidget::SetFilterParams ()
	{
		Filter_.SetFilterString (SearchWidget_->GetEdit ().text ());
	}

	void SummaryWidget::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
	{
		QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
		QMenu *sourceMenu = current.data (+CustomDataRoles::ContextMenu).value<QMenu*> ();
		if (!sourceMenu)
			return;

		QMenu *menu = new QMenu ();
		connect (menu,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleActionTriggered (QAction*)));
		menu->setAttribute (Qt::WA_DeleteOnClose, true);
		menu->addActions (CreateProxyActions (sourceMenu->actions (), menu));
		menu->setTitle (sourceMenu->title ());
		menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
	}

	void SummaryWidget::syncSelection (const QModelIndex& current)
	{
		QItemSelectionModel *selm = Ui_.PluginsTasksTree_->selectionModel ();
		const QModelIndex& now = selm->currentIndex ();
		if (current != now ||
				(now.isValid () &&
					!selm->rowIntersectsSelection (now.row (), QModelIndex ())))
		{
			selm->select (now, QItemSelectionModel::ClearAndSelect |
					QItemSelectionModel::Rows);
			updatePanes (now, current);
		}
	}
}
