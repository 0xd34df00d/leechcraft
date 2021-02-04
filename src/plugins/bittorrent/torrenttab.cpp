/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrenttab.h"
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QMenu>
#include <libtorrent/session.hpp>
#include <libtorrent/announce_entry.hpp>
#include <util/tags/tagscompleter.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/gui/lineeditbuttonmanager.h>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "movetorrentfiles.h"
#include "tabviewproxymodel.h"
#include "xmlsettingsmanager.h"
#include "types.h"
#include "sessionholder.h"
#include "ltutils.h"
#include "listactions.h"

namespace LC
{
namespace BitTorrent
{
	namespace
	{
		class TorrentsListDelegate : public QStyledItemDelegate
		{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				if (index.column () != Core::ColumnProgress)
				{
					QStyledItemDelegate::paint (painter, option, index);
					return;
				}

				const auto progress = index.data (Roles::SortRole).toDouble ();

				QStyleOptionProgressBar pbo;
				pbo.rect = option.rect;
				pbo.minimum = 0;
				pbo.maximum = 1000;
				pbo.progress = std::round (progress * 1000);
				pbo.state = option.state;
				pbo.text = Util::ElideProgressBarText (index.data ().toString (), option);
				pbo.textVisible = true;
				QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
			}
		};
	}

	TorrentTab::TorrentTab (SessionHolder& holder, const TabClassInfo& tc, QObject *mt)
	: Holder_ { holder }
	, TC_ { tc }
	, ParentMT_ { mt }
	, Actions_ { new ListActions { { holder, [this] { return this; } }, this } }
	, ViewFilter_ { new TabViewProxyModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.Tabs_->SetDependencies ({
				Core::Instance ()->GetSessionSettingsManager (),
				holder
			});

		ViewFilter_->setDynamicSortFilter (true);
		ViewFilter_->setSortRole (Roles::SortRole);
		ViewFilter_->setSourceModel (Core::Instance ());

		Ui_.TorrentsView_->setItemDelegate (new TorrentsListDelegate (Ui_.TorrentsView_));

		Ui_.TorrentsView_->setModel (ViewFilter_);
		connect (Ui_.TorrentsView_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				[this] (const QModelIndex& index)
				{
					Ui_.Tabs_->SetCurrentIndex (ViewFilter_->mapToSource (index).row ());
				});
		connect (Ui_.TorrentsView_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				[this]
				{
					const auto& rows = Ui_.TorrentsView_->selectionModel ()->selectedRows ();
					const auto& idxs = Util::Map (rows,
							[] (const QModelIndex& idx) { return idx.data (Roles::HandleIndex).toInt (); });
					Ui_.Tabs_->SetSelectedIndices (idxs);
				});
		Ui_.TorrentsView_->sortByColumn (Core::ColumnID, Qt::SortOrder::AscendingOrder);

		QHeaderView *header = Ui_.TorrentsView_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (Core::Columns::ColumnID,
				fm.horizontalAdvance ("999"));
		header->resizeSection (Core::Columns::ColumnName,
				fm.horizontalAdvance ("boardwalk.empire.s03e02.hdtv.720p.ac3.rus.eng.novafilm.tv.mkv") * 1.3);

		auto buttonMgr = new Util::LineEditButtonManager (Ui_.SearchLine_);
		new Util::TagsCompleter (Ui_.SearchLine_);
		Ui_.SearchLine_->AddSelector (buttonMgr);
		new Util::ClearLineEditAddon (Core::Instance ()->GetProxy (), Ui_.SearchLine_, buttonMgr);
		connect (Ui_.SearchLine_,
				SIGNAL (textChanged (QString)),
				ViewFilter_,
				SLOT (setFilterFixedString (QString)));

		connect (Ui_.TorrentStateFilter_,
				SIGNAL (currentIndexChanged (int)),
				ViewFilter_,
				SLOT (setStateFilterMode (int)));

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
		emit removeTab (this);
	}

	QToolBar* TorrentTab::GetToolBar () const
	{
		return Actions_->GetToolbar ();
	}

	void TorrentTab::SetCurrentTorrent (int row)
	{
		const auto& srcIdx = Core::Instance ()->index (row, 0);
		Ui_.TorrentsView_->setCurrentIndex (ViewFilter_->mapFromSource (srcIdx));
	}
}
}
