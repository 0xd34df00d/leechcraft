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
#include <QSortFilterProxyModel>
#include <util/gui/clearlineeditaddon.h>
#include <util/sll/slotclosure.h>
#include <util/sll/prelude.h>
#include <util/util.h>
#include "filesviewdelegate.h"
#include "core.h"
#include "torrentfilesmodel.h"

namespace LC
{
namespace BitTorrent
{
	namespace
	{
		class FilesProxyModel : public QSortFilterProxyModel
		{
		public:
			FilesProxyModel (QObject *parent)
			: QSortFilterProxyModel { parent }
			{
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const override
			{
				const auto& idx = sourceModel ()->index (row, TorrentFilesModel::ColumnPath, parent);

				if (idx.data ().toString ().contains (filterRegExp ().pattern (), Qt::CaseInsensitive))
					return true;

				const auto rc = sourceModel ()->rowCount (idx);
				for (int i = 0; i < rc; ++i)
					if (filterAcceptsRow (i, idx))
						return true;

				return false;
			}
		};
	}

	TorrentTabFilesWidget::TorrentTabFilesWidget (QWidget *parent)
	: QWidget { parent }
	, ProxyModel_ { new FilesProxyModel { this } }
	{
		Ui_.setupUi (this);

		new Util::ClearLineEditAddon { Core::Instance ()->GetProxy (), Ui_.SearchLine_ };

		ProxyModel_->setSortRole (TorrentFilesModel::RoleSort);
		Ui_.FilesView_->setItemDelegate (new FilesViewDelegate (Ui_.FilesView_));
		Ui_.FilesView_->setModel (ProxyModel_);

		connect (Ui_.FilesView_->selectionModel (),
				SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (currentFileChanged (const QModelIndex&)));

		currentFileChanged ({});

		connect (Ui_.SearchLine_,
				SIGNAL (textChanged (QString)),
				ProxyModel_,
				SLOT (setFilterFixedString (QString)));
		connect (Ui_.SearchLine_,
				SIGNAL (textChanged (QString)),
				Ui_.FilesView_,
				SLOT (expandAll ()));
	}

	void TorrentTabFilesWidget::SetCurrentIndex (int index)
	{
		ProxyModel_->setSourceModel (nullptr);
		delete CurrentFilesModel_;

		Ui_.SearchLine_->clear ();

		CurrentFilesModel_ = new TorrentFilesModel { index };
		connect (Core::Instance (),
				&Core::fileRenamed,
				CurrentFilesModel_,
				&TorrentFilesModel::HandleFileRenamed);
		ProxyModel_->setSourceModel (CurrentFilesModel_);
		QTimer::singleShot (0,
				Ui_.FilesView_,
				SLOT (expandAll ()));

		Ui_.SearchLine_->setVisible (Core::Instance ()->GetTorrentFiles (index).size () > 1);

		const auto& fm = Ui_.FilesView_->fontMetrics ();
		Ui_.FilesView_->header ()->resizeSection (0,
				fm.horizontalAdvance ("some very long file name or a directory name in a torrent file"));
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

	void TorrentTabFilesWidget::currentFileChanged (const QModelIndex& index)
	{
		Ui_.FilePriorityRegulator_->setEnabled (index.isValid ());

		if (!index.isValid ())
		{
			Ui_.FilePath_->setText ("");
			Ui_.FileProgress_->setText ("");
			Ui_.FilePriorityRegulator_->blockSignals (true);
			Ui_.FilePriorityRegulator_->setValue (0);
			Ui_.FilePriorityRegulator_->blockSignals (false);
		}
		else
		{
			auto path = index.data (TorrentFilesModel::RoleFullPath).toString ();
			path = QApplication::fontMetrics ()
				.elidedText (path,
						Qt::ElideLeft,
						Ui_.FilePath_->width ());
			Ui_.FilePath_->setText (path);

			auto sindex = index.sibling (index.row (),
					TorrentFilesModel::ColumnProgress);
			double progress = sindex.data (TorrentFilesModel::RoleProgress).toDouble ();
			qint64 size = sindex.data (TorrentFilesModel::RoleSize).toLongLong ();
			qint64 done = progress * size;
			Ui_.FileProgress_->setText (tr ("%1% (%2 of %3)")
					.arg (progress * 100, 0, 'f', 1)
					.arg (Util::MakePrettySize (done))
					.arg (Util::MakePrettySize (size)));

			Ui_.FilePriorityRegulator_->blockSignals (true);
			if (index.model ()->rowCount (index))
				Ui_.FilePriorityRegulator_->setValue (1);
			else
			{
				auto prindex = index.sibling (index.row (),
						TorrentFilesModel::ColumnPriority);
				int priority = prindex.data ().toInt ();
				Ui_.FilePriorityRegulator_->setValue (priority);
			}
			Ui_.FilePriorityRegulator_->blockSignals (false);
		}
	}

	void TorrentTabFilesWidget::on_FilePriorityRegulator__valueChanged (int prio)
	{
		for (auto idx : GetSelectedIndexes ())
		{
			idx = idx.sibling (idx.row (), TorrentFilesModel::ColumnPriority);
			Ui_.FilesView_->model ()->setData (idx, prio);
		}
	}

	void TorrentTabFilesWidget::on_FilesView__customContextMenuRequested (const QPoint& pos)
	{
		const auto itm = Core::Instance ()->GetProxy ()->GetIconThemeManager ();

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
			const auto openAct = menu.addAction (openActName);
			openAct->setIcon (itm->GetIcon ("document-open"));
			new Util::SlotClosure<Util::DeleteLaterPolicy>
			{
				[openable, this]
				{
					for (const auto& idx : openable)
						CurrentFilesModel_->HandleFileActivated (ProxyModel_->mapToSource (idx));
				},
				openAct,
				SIGNAL (triggered ()),
				openAct
			};

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
					return idxPair.first
							.sibling (idxPair.first.row (), TorrentFilesModel::ColumnPriority);
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

				const auto act = subMenu->addAction (QString::number (prio) + " " + descr.second);

				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[this, prio, priorityRoots]
					{
						for (const auto& idx : priorityRoots)
							ProxyModel_->setData (idx, prio);
					},
					act,
					SIGNAL (triggered ()),
					act
				};
			}
		}

		menu.addAction (tr ("Expand all"), Ui_.FilesView_, SLOT (expandAll ()));
		menu.addAction (tr ("Collapse all"), Ui_.FilesView_, SLOT (collapseAll ()));

		menu.exec (Ui_.FilesView_->viewport ()->mapToGlobal (pos));
	}
}
}
