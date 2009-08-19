/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "piecesmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			bool PiecesModel::Info::operator== (const Info& other) const
			{
				return Index_ == other.Index_;
			}
			
			PiecesModel::PiecesModel (QObject *parent)
			: QAbstractItemModel (parent)
			{
				Headers_ << tr ("Index") << tr ("Speed") << tr ("State");
			}
			
			PiecesModel::~PiecesModel ()
			{
			}
			
			int PiecesModel::columnCount (const QModelIndex&) const
			{
				return Headers_.size ();
			}
			
			QVariant PiecesModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid () || role != Qt::DisplayRole)
					return QVariant ();
				
				switch (index.column ())
				{
					case 0:
						return QString::number (Pieces_.at (index.row ()).Index_);
					case 1:
						switch (Pieces_.at (index.row ()).State_)
						{
							case libtorrent::partial_piece_info::none:
								return tr ("None");
							case libtorrent::partial_piece_info::slow:
								return tr ("Slow");
							case libtorrent::partial_piece_info::medium:
								return tr ("Medium");
							case libtorrent::partial_piece_info::fast:
								return tr ("Fast");
							default:
								return QVariant ();
						}
					case 2:
						return QString ("%1/%2").arg (Pieces_.at (index.row ()).FinishedBlocks_).arg (Pieces_.at (index.row ()).TotalBlocks_);
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
					return QModelIndex ();
			
				return createIndex (row, column);
			} 
			
			QVariant PiecesModel::headerData (int column, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Vertical)
					return QVariant ();
			
				if (role != Qt::DisplayRole)
					return QVariant ();
			
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
			
			void PiecesModel::Clear ()
			{
				if (!Pieces_.size ())
					return;
			
				beginRemoveRows (QModelIndex (), 0, Pieces_.size () - 1);
				Pieces_.clear ();
				endRemoveRows ();
			}
			
			void PiecesModel::Update (const std::vector<libtorrent::partial_piece_info>& queue)
			{
				QList<Info> pieces2Insert;
				QMap<int, int> index2position;
			
				for (int i = 0; i < Pieces_.size (); ++i)
					index2position [Pieces_.at (i).Index_] = i;
			
				// Update
				for (size_t i = 0; i < queue.size (); ++i)
				{
					libtorrent::partial_piece_info ppi = queue [i];
			
					bool found = false;
					for (int j = 0; j < Pieces_.size (); ++j)
						if (Pieces_.at (j).Index_ == ppi.piece_index)
						{
							index2position.remove (Pieces_.at (j).Index_);
							Pieces_ [j].State_ = ppi.piece_state;
							Pieces_ [j].FinishedBlocks_ = 0;
							for (int k = 0; k < ppi.blocks_in_piece; ++k)
								Pieces_ [j].FinishedBlocks_ += (ppi.blocks [k].state == libtorrent::block_info::finished);
							found = true;
							emit dataChanged (index (j, 1), index (j, 2));
							break;
						}
					if (found)
						continue;
			
					Info info;
					info.Index_ = ppi.piece_index;
					info.State_ = ppi.piece_state;
					info.TotalBlocks_ = ppi.blocks_in_piece;
					info.FinishedBlocks_ = 0;
					for (int k = 0; k < ppi.blocks_in_piece; ++k)
						info.FinishedBlocks_ += (ppi.blocks [k].state == libtorrent::block_info::finished);
					pieces2Insert << info;
				}
			
				// Remove
				QList<int> values = index2position.values ();
				qSort (values.begin (), values.end (), qGreater<int> ());
				for (int i = 0; i < values.size (); ++i)
				{
					beginRemoveRows (QModelIndex (), values.at (i), values.at (i));
					Pieces_.removeAt (values.at (i));
					endRemoveRows ();
				}
			
				// Insert new
				if (pieces2Insert.size ())
				{
					beginInsertRows (QModelIndex (), Pieces_.size (), Pieces_.size () + pieces2Insert.size () - 1);
					Pieces_ += pieces2Insert;
					endInsertRows ();
				}
			}
			
		};
	};
};

