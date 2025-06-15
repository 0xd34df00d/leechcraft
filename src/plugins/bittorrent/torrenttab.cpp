/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrenttab.h"
#include <cmath>
#include <QToolBar>
#include <QMenu>
#include <util/tags/tagscompleter.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/gui/lineeditbuttonmanager.h>
#include <util/gui/progressdelegate.h>
#include <util/gui/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "movetorrentfiles.h"
#include "tabviewproxymodel.h"
#include "xmlsettingsmanager.h"
#include "types.h"
#include "ltutils.h"
#include "listactions.h"

namespace LC::BitTorrent
{
	TorrentTab::TorrentTab (const Dependencies& deps, const TabClassInfo& tc, QObject *mt)
	: D_ { deps }
	, TC_ { tc }
	, ParentMT_ { mt }
	, Actions_ { new ListActions { { deps.Session_, [this] { return this; } }, this } }
	, ViewFilter_ { new TabViewProxyModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.Tabs_->SetDependencies ({
				deps.Dispatcher_,
				deps.Model_,
				deps.SSM_,
				deps.Session_,
			});

		ViewFilter_->setDynamicSortFilter (true);
		ViewFilter_->setSortRole (Roles::SortRole);
		ViewFilter_->setSourceModel (&deps.Model_);

		Ui_.TorrentsView_->setItemDelegateForColumn (Columns::ColumnProgress,
				new Util::ProgressDelegate
				{
					[] (const QModelIndex& index) -> Util::ProgressDelegate::Progress
					{
						const auto precision = 1000;
						const auto progress = std::round (index.data (Roles::TorrentProgress).toDouble () * precision);
						return
						{
							.Maximum_ = precision,
							.Progress_ = static_cast<int> (progress),
							.Text_ = index.data ().toString (),
						};
					},
					Ui_.TorrentsView_
				});

		Ui_.TorrentsView_->setModel (ViewFilter_);
		connect (Ui_.TorrentsView_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				[this] (const QModelIndex& index) { Ui_.Tabs_->SetCurrentIndex (ViewFilter_->mapToSource (index)); });
		connect (Ui_.TorrentsView_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				[this]
				{
					const auto& rows = Ui_.TorrentsView_->selectionModel ()->selectedRows ();
					Ui_.Tabs_->SetSelectedIndices (rows);
				});
		Ui_.TorrentsView_->sortByColumn (Columns::ColumnID, Qt::SortOrder::AscendingOrder);

		QHeaderView *header = Ui_.TorrentsView_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (Columns::ColumnID,
				fm.horizontalAdvance (QStringLiteral ("999")));
		header->resizeSection (Columns::ColumnName,
				fm.horizontalAdvance (QStringLiteral ("boardwalk.empire.s03e02.hdtv.720p.ac3.rus.eng.novafilm.tv.mkv")) * 1.3);

		auto buttonMgr = new Util::LineEditButtonManager (Ui_.SearchLine_);
		new Util::TagsCompleter (Ui_.SearchLine_);
		Ui_.SearchLine_->AddSelector (buttonMgr);
		new Util::ClearLineEditAddon (GetProxyHolder (), Ui_.SearchLine_, buttonMgr);
		connect (Ui_.SearchLine_,
				&QLineEdit::textChanged,
				ViewFilter_,
				&TabViewProxyModel::SetFilterString);

		connect (Ui_.TorrentStateFilter_,
				&QComboBox::currentIndexChanged,
				ViewFilter_,
				&TabViewProxyModel::SetStateFilterMode);

		auto selModel = Ui_.TorrentsView_->selectionModel ();
		connect (selModel,
				&QItemSelectionModel::currentRowChanged,
				Actions_,
				&ListActions::SetCurrentIndex);
		connect (selModel,
				&QItemSelectionModel::selectionChanged,
				Actions_,
				[this, selModel] { Actions_->SetCurrentSelection (selModel->selectedRows ()); });

		connect (Ui_.TorrentsView_,
				&QTreeView::customContextMenuRequested,
				[this] (QPoint at)
				{
					const auto& globalAt = Ui_.TorrentsView_->viewport ()->mapToGlobal (at);
					Actions_->MakeContextMenu ()->popup (globalAt);
				});
		/*
		Toolbar_->addSeparator ();
		DownSelectorAction_ = new SpeedSelectorAction ("Down", this);
		DownSelectorAction_->handleSpeedsChanged ();
		Toolbar_->addAction (DownSelectorAction_);
		UpSelectorAction_ = new SpeedSelectorAction ("Up", this);
		UpSelectorAction_->handleSpeedsChanged ();
		Toolbar_->addAction (UpSelectorAction_);

		connect (DownSelectorAction_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleFastSpeedComboboxes ()));
		connect (UpSelectorAction_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleFastSpeedComboboxes ()));
				*/
	}

	TabClassInfo TorrentTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* TorrentTab::ParentMultiTabs ()
	{
		return ParentMT_;
	}

	void TorrentTab::Remove ()
	{
		emit removeTab ();
	}

	QToolBar* TorrentTab::GetToolBar () const
	{
		return Actions_->GetToolbar ();
	}

	void TorrentTab::SetCurrentTorrent (const QModelIndex& idx)
	{
		Ui_.TorrentsView_->setCurrentIndex (ViewFilter_->mapFromSource (idx));
	}
}
