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

#include "urlcompletionmodel.h"
#include <stdexcept>
#include <QUrl>
#include <QtDebug>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			URLCompletionModel::URLCompletionModel (QObject *parent)
			: QAbstractItemModel (parent)
			, Valid_ (false)
			{
			}
			
			URLCompletionModel::~URLCompletionModel ()
			{
			}
			
			int URLCompletionModel::columnCount (const QModelIndex&) const
			{
				return 1;
			}
			
			QVariant URLCompletionModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				if (role == Qt::DisplayRole)
					return Items_ [index.row ()].Title_ + " [" + Items_ [index.row ()].URL_ + "]";
				else if (role == Qt::DecorationRole)
					return Core::Instance ().GetIcon (QUrl (Items_ [index.row ()].URL_));
				else if (role == Qt::EditRole)
					return Base_ + index.row ();
				else if (role == RoleURL)
					return Items_ [index.row ()].URL_;
				else
					return QVariant ();
			}
			
			Qt::ItemFlags URLCompletionModel::flags (const QModelIndex&) const
			{
				return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}
			
			QVariant URLCompletionModel::headerData (int, Qt::Orientation, int) const
			{
				return QVariant ();
			}
			
			QModelIndex URLCompletionModel::index (int row, int column,
					const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex URLCompletionModel::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int URLCompletionModel::rowCount (const QModelIndex& index) const
			{
				if (index.isValid ())
					return 0;
			
				return Items_.size ();
			}
			
			void URLCompletionModel::setBase (const QString& str)
			{
				Valid_ = false;
				Base_ = str;
			
				Populate ();
			}
			
			void URLCompletionModel::handleItemAdded (const HistoryItem&)
			{
				Valid_ = false;
				Populate ();
			}
			
			void URLCompletionModel::Populate ()
			{
				if (!Valid_)
				{
					Valid_ = true;
			
					int size = Items_.size () - 1;
					if (size > 0)
						beginRemoveRows (QModelIndex (), 0, size);
					Items_.clear ();
					if (size > 0)
						endRemoveRows ();
			
					try
					{
						Core::Instance ().GetStorageBackend ()->LoadResemblingHistory (Base_, Items_);
					}
					catch (const std::runtime_error& e)
					{
						qWarning () << Q_FUNC_INFO << e.what ();
						Valid_ = false;
					}
			
					size = Items_.size () - 1;
					beginInsertRows (QModelIndex (), 0, size);
					endInsertRows ();
				}
			}
		};
	};
};

