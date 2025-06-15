/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrenttabfileswidget.h"
#include <QMenu>
#include <QTimer>
#include <libtorrent/torrent_handle.hpp>
#include <util/gui/clearlineeditaddon.h>
#include <util/gui/progressdelegate.h>
#include <util/models/fixedstringfilterproxymodel.h>
#include <util/sll/prelude.h>
#include <util/util.h>
#include "filesviewdelegate.h"
#include "ltutils.h"
#include "torrentfilesmodel.h"

namespace LC::BitTorrent
{
	namespace
	{
		class FilesProxyModel final : public Util::FixedStringFilterProxyModel
		{
		public:
			using FixedStringFilterProxyModel::FixedStringFilterProxyModel;
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const override
			{
				const auto& idx = sourceModel ()->index (row, TorrentFilesModel::ColumnPath, parent);

				if (idx.data ().toString ().contains (FilterFixedString_, Qt::CaseInsensitive))
					return true;

				const auto rc = sourceModel ()->rowCount (idx);
				for (int i = 0; i < rc; ++i)
					if (filterAcceptsRow (i, idx))
						return true;

				return false;
			}
		};

		Util::ProgressDelegate::Progress GetFileProgress (const QModelIndex& idx)
		{
			const auto progress = idx.data (TorrentFilesModel::RoleProgress).toDouble () * 100;
			const auto percentage = std::clamp (static_cast<int> (progress * 100), 0, 100);
			const qlonglong size = idx.data (TorrentFilesModel::RoleSize).toLongLong ();
			const qlonglong done = progress * size;
			const auto& text = TorrentTabFilesWidget::tr ("%1% (%2 of %3)")
					.arg (percentage)
					.arg (Util::MakePrettySize (done), Util::MakePrettySize (size));
			return
			{
				.Maximum_ = 100,
				.Progress_ = percentage,
				.Text_ = text,
			};
		}
	}

	TorrentTabFilesWidget::TorrentTabFilesWidget (QWidget *parent)
	: QWidget { parent }
	, ProxyModel_ { std::make_unique<FilesProxyModel> (this) }
	{
		Ui_.setupUi (this);

		new Util::ClearLineEditAddon { GetProxyHolder (), Ui_.SearchLine_ };

		ProxyModel_->setSortRole (TorrentFilesModel::RoleSort);
		Ui_.FilesView_->setItemDelegate (new FilesViewDelegate (Ui_.FilesView_));
		Ui_.FilesView_->setItemDelegateForColumn (TorrentFilesModel::ColumnProgress,
				new Util::ProgressDelegate { &GetFileProgress, Ui_.FilesView_ });
		Ui_.FilesView_->setModel (&*ProxyModel_);

		connect (Ui_.FilesView_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				this,
				&TorrentTabFilesWidget::HandleFileSelected);

		HandleFileSelected ({});

		connect (Ui_.SearchLine_,
				&QLineEdit::textChanged,
				[this] (const QString& text)
				{
					ProxyModel_->setFilterFixedString (text);
					Ui_.FilesView_->expandAll ();
				});

		connect (Ui_.FilePriorityRegulator_,
				&QSpinBox::valueChanged,
				[this] (int prio)
				{
					for (auto idx : GetSelectedIndexes ())
					{
						idx = idx.siblingAtColumn (TorrentFilesModel::ColumnPriority);
						Ui_.FilesView_->model ()->setData (idx, prio);
					}
				});

		connect (Ui_.FilesView_,
				&QTreeView::customContextMenuRequested,
				this,
				&TorrentTabFilesWidget::ShowContextMenu);
	}

	TorrentTabFilesWidget::~TorrentTabFilesWidget () = default;

	void TorrentTabFilesWidget::SetAlertDispatcher (AlertDispatcher& dispatcher)
	{
		AlertDispatcher_ = &dispatcher;
	}

	void TorrentTabFilesWidget::SetCurrentIndex (const QModelIndex& index)
	{
		ProxyModel_->setSourceModel (nullptr);
		CurrentFilesModel_.reset ();

		Ui_.SearchLine_->clear ();

		const auto& handle = GetTorrentHandle (index);
		if (!handle.is_valid ())
			return;

		CurrentFilesModel_ = std::make_unique<TorrentFilesModel> (handle, *AlertDispatcher_);
		ProxyModel_->setSourceModel (&*CurrentFilesModel_);
		QTimer::singleShot (0,
				Ui_.FilesView_,
				&QTreeView::expandAll);

		const auto filesCount = GetFilesCount (handle);
		Ui_.SearchLine_->setVisible (filesCount > 1);

		const auto& fm = Ui_.FilesView_->fontMetrics ();
		Ui_.FilesView_->header ()->resizeSection (0,
				fm.horizontalAdvance (QStringLiteral ("some very long file name or a directory name in a torrent file")));
	}

	QList<QModelIndex> TorrentTabFilesWidget::GetSelectedIndexes () const
	{
		const auto selModel = Ui_.FilesView_->selectionModel ();

		const auto& current = selModel->currentIndex ();
		auto selected = selModel->selectedRows ();
		if (!selected.contains (current))
			selected.append (current);
		return selected;
	}

	void TorrentTabFilesWidget::HandleFileSelected (const QModelIndex& index)
	{
		Ui_.FilePriorityRegulator_->setEnabled (index.isValid ());

		if (!index.isValid ())
		{
			Ui_.FilePath_->setText ({});
			Ui_.FileProgress_->setText ({});
			Ui_.FilePriorityRegulator_->blockSignals (true);
			Ui_.FilePriorityRegulator_->setValue (0);
			Ui_.FilePriorityRegulator_->blockSignals (false);
		}
		else
		{
			auto path = index.data (TorrentFilesModel::RoleFullPath).toString ();
			path = fontMetrics ().elidedText (path,
					Qt::ElideLeft,
					Ui_.FilePath_->width ());
			Ui_.FilePath_->setText (path);

			auto sindex = index.siblingAtColumn (TorrentFilesModel::ColumnProgress);
			double progress = sindex.data (TorrentFilesModel::RoleProgress).toDouble ();
			qint64 size = sindex.data (TorrentFilesModel::RoleSize).toLongLong ();
			qint64 done = progress * size;
			Ui_.FileProgress_->setText (tr ("%1% (%2 of %3)")
					.arg (progress * 100, 0, 'f', 1)
					.arg (Util::MakePrettySize (done),
						  Util::MakePrettySize (size)));

			Ui_.FilePriorityRegulator_->blockSignals (true);
			if (index.model ()->rowCount (index))
				Ui_.FilePriorityRegulator_->setValue (1);
			else
			{
				auto prindex = index.siblingAtColumn (TorrentFilesModel::ColumnPriority);
				int priority = prindex.data ().toInt ();
				Ui_.FilePriorityRegulator_->setValue (priority);
			}
			Ui_.FilePriorityRegulator_->blockSignals (false);
		}
	}

	void TorrentTabFilesWidget::ShowContextMenu (const QPoint& pos)
	{
		const auto itm = GetProxyHolder ()->GetIconThemeManager ();

		QMenu menu;

		const auto& selected = GetSelectedIndexes ();
		const auto& openable = Util::Filter (selected,
				[] (const QModelIndex& idx)
				{
					const auto progress = idx.data (TorrentFilesModel::RoleProgress).toDouble ();
					return idx.model ()->rowCount (idx) ||
							std::abs (progress - 1) < std::numeric_limits<double>::epsilon ();
				});
		if (!openable.isEmpty ())
		{
			const auto& openActName = openable.size () == 1 ?
					tr ("Open file") :
					tr ("Open %n file(s)", 0, openable.size ());
			const auto openAct = menu.addAction (openActName,
					this,
					[openable, this]
					{
						for (const auto& idx : openable)
							CurrentFilesModel_->HandleFileActivated (ProxyModel_->mapToSource (idx));
					});
			openAct->setIcon (itm->GetIcon (QStringLiteral ("document-open")));

			menu.addSeparator ();
		}

		const auto& cachedRoots = Util::Map (selected,
				[] (const QModelIndex& idx)
				{
					return qMakePair (idx, idx.data (TorrentFilesModel::RoleFullPath).toString ());
				});
		const auto& priorityRoots = Util::Map (Util::Filter (cachedRoots,
					[&cachedRoots] (const QPair<QModelIndex, QString>& idxPair)
					{
						return std::none_of (cachedRoots.begin (), cachedRoots.end (),
								[&idxPair] (const QPair<QModelIndex, QString>& existing)
								{
									return idxPair.first != existing.first &&
											idxPair.second.startsWith (existing.second);
								});
					}),
				[] (const QPair<QModelIndex, QString>& idxPair)
				{
					return idxPair.first.siblingAtColumn (TorrentFilesModel::ColumnPriority);
				});
		if (!priorityRoots.isEmpty ())
		{
			const auto subMenu = menu.addMenu (tr ("Change priority"));
			const QList<QPair<int, QString>> descrs
			{
				{ 0, tr ("File is not downloaded.") },
				{ 1, tr ("Normal priority, download order depends on availability.") },
				{ 2, tr ("Pieces are preferred over the pieces with same availability.") },
				{ 3, tr ("Empty pieces are preferred just as much as partial pieces.") },
				{ 4, tr ("Empty pieces are preferred over partial pieces with the same availability.") },
				{ 5, tr ("Same as previous.") },
				{ 6, tr ("Pieces are considered to have highest availability.") },
				{ 7, tr ("Maximum file priority.") }
			};

			for (const auto& descr : descrs)
			{
				const auto prio = descr.first;

				subMenu->addAction (QString::number (prio) + " " + descr.second,
						this,
						[this, prio, priorityRoots]
						{
							for (const auto& idx : priorityRoots)
								ProxyModel_->setData (idx, prio);
						});
			}
		}

		menu.addAction (tr ("Expand all"), Ui_.FilesView_, &QTreeView::expandAll);
		menu.addAction (tr ("Collapse all"), Ui_.FilesView_, &QTreeView::collapseAll);

		menu.exec (Ui_.FilesView_->viewport ()->mapToGlobal (pos));
	}
}
