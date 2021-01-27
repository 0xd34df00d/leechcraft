/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "piecesmodel.h"
#include <QTimer>
#include "core.h"

namespace LC::BitTorrent
{
	bool PiecesModel::Info::operator== (const Info& other) const
	{
		return Index_ == other.Index_;
	}

	PiecesModel::PiecesModel (int index, QObject *parent)
	: QAbstractItemModel (parent)
	, Index_ (index)
	{
		Headers_ << tr ("Index") << tr ("State");
		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (update ()));
		timer->start (2000);
		QTimer::singleShot (0,
				this,
				SLOT (update ()));
	}

	int PiecesModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant PiecesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () || role != Qt::DisplayRole)
			return {};

		switch (index.column ())
		{
		case 0:
			return QString::number (Pieces_.at (index.row ()).Index_);
		case 1:
			return QStringLiteral ("%1/%2")
					.arg (Pieces_.at (index.row ()).FinishedBlocks_)
					.arg (Pieces_.at (index.row ()).TotalBlocks_);
		default:
			return QVariant ();
		}
	}

	Qt::ItemFlags PiecesModel::flags (const QModelIndex&) const
	{
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}

	bool PiecesModel::hasChildren (const QModelIndex& index) const
	{
		return !index.isValid ();
	}

	QModelIndex PiecesModel::index (int row, int column, const QModelIndex&) const
	{
		if (!hasIndex (row, column))
			return {};

		return createIndex (row, column);
	}

	QVariant PiecesModel::headerData (int column, Qt::Orientation orient, int role) const
	{
		if (orient == Qt::Vertical || role != Qt::DisplayRole)
			return {};

		return Headers_ [column];
	}

	QModelIndex PiecesModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int PiecesModel::rowCount (const QModelIndex& index) const
	{
		if (index.isValid ())
			return 0;

		return Pieces_.size ();
	}

	void PiecesModel::Update ()
	{
		const auto& handle = Core::Instance ()->GetTorrentHandle (Index_);
		if (!handle.is_valid ())
			return;

		std::vector<libtorrent::partial_piece_info> queue;
		handle.get_download_queue (queue);

		QList<Info> pieces2Insert;
		QMap<int, int> index2position;

		const int initSize = Pieces_.size ();
		for (int i = 0; i < initSize; ++i)
			index2position [Pieces_.at (i).Index_] = i;

		// Update
		for (auto & ppi : queue)
		{
			bool found = false;

			for (int j = 0; j < initSize; ++j)
				if (Pieces_.at (j).Index_ == ppi.piece_index)
				{
					index2position.remove (Pieces_.at (j).Index_);
					Pieces_ [j].FinishedBlocks_ = ppi.finished;
					found = true;
					emit dataChanged (index (j, 1), index (j, 2));
					break;
				}
			if (found)
				continue;

			Info info;
			info.Index_ = ppi.piece_index;
			info.TotalBlocks_ = ppi.blocks_in_piece;
			info.FinishedBlocks_ = ppi.finished;
			pieces2Insert << info;
		}

		// Remove
		auto values = index2position.values ();
		std::sort (values.begin (), values.end (), std::greater<> ());
		for (auto value : values)
		{
			beginRemoveRows (QModelIndex (), value, value);
			Pieces_.removeAt (value);
			endRemoveRows ();
		}

		// Insert new
		if (!pieces2Insert.isEmpty ())
		{
			beginInsertRows (QModelIndex (), Pieces_.size (), Pieces_.size () + pieces2Insert.size () - 1);
			Pieces_ += pieces2Insert;
			endInsertRows ();
		}
	}
}
