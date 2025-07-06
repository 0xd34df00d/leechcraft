/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "summarywidget.h"
#include <QTimer>
#include <QComboBox>
#include <QMenu>
#include <QToolBar>
#include <QMainWindow>
#include <QWidgetAction>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/imwproxy.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/sll/qtutil.h>
#include "core.h"
#include "summary.h"
#include "summarytagsfilter.h"
#include "modeldelegate.h"

Q_DECLARE_METATYPE (QMenu*)

namespace LC
{
namespace Summary
{
	QObject *SummaryWidget::S_ParentMultiTabs_ = 0;

	class SearchWidget : public QWidget
	{
		QLineEdit *Edit_;
	public:
		SearchWidget (SummaryWidget *summary)
		: Edit_ (new QLineEdit)
		{
			auto lay = new QHBoxLayout;
			setLayout (lay);

			Edit_->setPlaceholderText (SummaryWidget::tr ("Search..."));
			Edit_->setMaximumWidth (400);
			Edit_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
			lay->addStretch ();
			lay->addWidget (Edit_, 0, Qt::AlignRight);
			new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Edit_);

			connect (Edit_,
					SIGNAL (textChanged (QString)),
					summary,
					SLOT (filterParametersChanged ()));
			connect (Edit_,
					SIGNAL (returnPressed ()),
					summary,
					SLOT (feedFilterParameters ()));
		}

		QString GetText () const
		{
			return Edit_->text ();
		}

		void SetText (const QString& text)
		{
			Edit_->setText (text);
		}
	};

	SummaryWidget::SummaryWidget (QWidget *parent)
	: QWidget (parent)
	, FilterTimer_ (new QTimer)
	, SearchWidget_ (CreateSearchWidget ())
	, Toolbar_ (new QToolBar)
	, Sorter_ (Core::Instance ().GetTasksModel ())
	{
		Toolbar_->setWindowTitle ("Summary");
		connect (Toolbar_.get (),
				SIGNAL (actionTriggered (QAction*)),
				this,
				SLOT (handleActionTriggered (QAction*)));

		Toolbar_->addWidget (SearchWidget_);

		Ui_.setupUi (this);
		Ui_.PluginsTasksTree_->setItemDelegate (new ModelDelegate (this));

		FilterTimer_->setSingleShot (true);
		FilterTimer_->setInterval (800);
		connect (FilterTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (feedFilterParameters ()));

		Ui_.ControlsDockWidget_->hide ();

		Ui_.PluginsTasksTree_->setModel (Sorter_);

		connect (Sorter_,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (checkDataChanged (QModelIndex, QModelIndex)));
		connect (Sorter_,
				SIGNAL (modelAboutToBeReset ()),
				this,
				SLOT (handleReset ()));
		connect (Sorter_,
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

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (const auto ijh : pm->GetAllCastableTo<IJobHolder*> ())
			if (const auto handler = ijh->CreateRepresentationHandler ())
				SrcModel2Handler_ [ijh->GetRepresentation ()] = handler;

		auto connectChange = [this] (auto signal, auto method)
		{
			connect (Ui_.PluginsTasksTree_->selectionModel (),
					signal,
					this,
					[this, method] (const QModelIndex& current, const QModelIndex& previous)
					{
						const auto& prevMapped = Core::Instance ().MapToSourceRecursively (previous);
						const auto& thisMapped = Core::Instance ().MapToSourceRecursively (current);

						if (prevMapped.isValid () && prevMapped.model () != thisMapped.model ())
							std::invoke (method, SrcModel2Handler_ [prevMapped.model ()], QModelIndex {});

						if (thisMapped.isValid ())
							std::invoke (method, SrcModel2Handler_ [thisMapped.model ()], thisMapped);
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
						const auto& mapped = Core::Instance ().MapToSourceRecursively (index);
						if (mapped.isValid ())
							std::invoke (method, SrcModel2Handler_ [mapped.model ()], mapped);
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
						const auto& mapped = Core::Instance ().MapToSourceRecursively (row);
						newSelections [mapped.model ()] << mapped;
					}

					for (const auto& [model, rows] : Util::Stlize (newSelections))
						SrcModel2Handler_ [model]->HandleSelectedRowsChanged (rows);

					QSet<const QAbstractItemModel*> curModels { newSelections.keyBegin (), newSelections.keyEnd () };
					for (const auto model : PreviouslySelectedModels_ - curModels)
						SrcModel2Handler_ [model]->HandleSelectedRowsChanged ({});
					PreviouslySelectedModels_ = curModels;
				});
	}

	SummaryWidget::~SummaryWidget ()
	{
		Toolbar_->clear ();

		QWidget *widget = Ui_.ControlsDockWidget_->widget ();
		Ui_.ControlsDockWidget_->setWidget (0);
		if (widget)
			widget->setParent (0);

		delete Sorter_;
	}

	void SummaryWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
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
		return S_ParentMultiTabs_;
	}

	TabClassInfo SummaryWidget::GetTabClassInfo () const
	{
		return qobject_cast<Summary*> (S_ParentMultiTabs_)->GetTabClasses ().first ();
	}

	SearchWidget* SummaryWidget::CreateSearchWidget ()
	{
		return new SearchWidget (this);
	}

	void SummaryWidget::ReinitToolbar ()
	{
		for (const auto action : Toolbar_->actions ())
		{
			auto wa = qobject_cast<QWidgetAction*> (action);
			if (!wa || wa->defaultWidget () != SearchWidget_)
			{
				Toolbar_->removeAction (action);
				delete action;
			}
			else if (wa->defaultWidget () != SearchWidget_)
				Toolbar_->removeAction (action);
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
		QByteArray result;
		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (1);
		return result;
	}

	QString SummaryWidget::GetTabRecoverName () const
	{
		return GetTabClassInfo ().VisibleName_;
	}

	QIcon SummaryWidget::GetTabRecoverIcon () const
	{
		return GetTabClassInfo ().Icon_;
	}

	void SummaryWidget::RestoreState (const QByteArray& data)
	{
		QDataStream in (data);
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version";
			return;
		}
	}

	void SummaryWidget::SetUpdatesEnabled (bool)
	{
		// TODO implement this
	}

	Ui::SummaryWidget SummaryWidget::GetUi () const
	{
		return Ui_;
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

	void SummaryWidget::checkDataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight)
	{
		const QModelIndex& cur = Ui_.PluginsTasksTree_->
				selectionModel ()->currentIndex ();
		if (topLeft.row () <= cur.row () && bottomRight.row () >= cur.row ())
			updatePanes (cur, cur);
	}

	void SummaryWidget::handleReset ()
	{
		Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void SummaryWidget::checkRowsToBeRemoved (const QModelIndex&, int begin, int end)
	{
		const QModelIndex& cur = Ui_.PluginsTasksTree_->
				selectionModel ()->currentIndex ();
		if (begin <= cur.row () && end >= cur.row ())
			Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void SummaryWidget::updatePanes (const QModelIndex& newIndex, const QModelIndex& oldIndex)
	{
		QToolBar *controls = Core::Instance ().GetControls (newIndex);
		QWidget *addiInfo = Core::Instance ().GetAdditionalInfo (newIndex);

		if (oldIndex.isValid () &&
				addiInfo != Ui_.ControlsDockWidget_->widget ())
			Ui_.ControlsDockWidget_->hide ();

		if (Core::Instance ().SameModel (newIndex, oldIndex))
			return;

		ReinitToolbar ();
		if (newIndex.isValid ())
		{
			if (controls)
			{
				for (const auto action : controls->actions ())
				{
					QString ai = action->property ("ActionIcon").toString ();
					if (!ai.isEmpty () &&
							action->icon ().isNull ())
						action->setIcon (Core::Instance ().GetProxy ()->
									GetIconThemeManager ()->GetIcon (ai));
				}

				const auto& proxies = CreateProxyActions (controls->actions (), Toolbar_.get ());
				Toolbar_->insertActions (Toolbar_->actions ().first (), proxies);
			}
			if (addiInfo != Ui_.ControlsDockWidget_->widget ())
				Ui_.ControlsDockWidget_->setWidget (addiInfo);

			if (addiInfo)
			{
				Ui_.ControlsDockWidget_->show ();
				Core::Instance ().GetProxy ()->GetIconThemeManager ()->
						UpdateIconset (addiInfo->findChildren<QAction*> ());
			}
		}
	}

	void SummaryWidget::filterParametersChanged ()
	{
		FilterTimer_->stop ();
		FilterTimer_->start ();
	}

	void SummaryWidget::filterReturnPressed ()
	{
		FilterTimer_->stop ();
		feedFilterParameters ();
	}

	void SummaryWidget::feedFilterParameters ()
	{
		Sorter_->SetFilterString (SearchWidget_->GetText ());
	}

	void SummaryWidget::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
	{
		QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
		QMenu *sourceMenu = current.data (RoleContextMenu).value<QMenu*> ();
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
}
