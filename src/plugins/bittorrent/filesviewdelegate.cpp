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

#include <QModelIndex>
#include <QSpinBox>
#include <QLineEdit>
#include <QtDebug>
#include <plugininterface/treeitem.h>
#include "filesviewdelegate.h"
#include "torrentfilesmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			namespace
			{
				bool HasChildren (const QModelIndex& index)
				{
					return index.model ()->rowCount (index.sibling (index.row (), 0));
				}
			};

			using LeechCraft::Util::TreeItem;
			
			FilesViewDelegate::FilesViewDelegate (QObject *parent)
			: QItemDelegate (parent)
			{
			}
			
			FilesViewDelegate::~FilesViewDelegate ()
			{
			}
			
			QWidget* FilesViewDelegate::createEditor (QWidget *parent,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				if (index.column () == 2 &&
						!HasChildren (index))
				{
					QSpinBox *box = new QSpinBox (parent);
					box->setRange (0, 7);
					return box;
				}
				else if (index.column () == 0 &&
						!HasChildren (index))
					return new QLineEdit (parent);
				else
					return QItemDelegate::createEditor (parent, option, index);
			}
			
			void FilesViewDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				QItemDelegate::paint (painter, option, index);
			}
			
			void FilesViewDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
			{
				if (index.column () == 2 &&
						!HasChildren (index))
					qobject_cast<QSpinBox*> (editor)->
						setValue (static_cast<TreeItem*> (index.internalPointer ())->
								Data (2).toInt ());
				else if (index.column () == 0 &&
						!HasChildren (index))
				{
					QVariant data = static_cast<TreeItem*> (index.internalPointer ())->
						Data (0, TorrentFilesModel::RawDataRole);
					qobject_cast<QLineEdit*> (editor)->setText (data.toString ());
				}
				else
					QItemDelegate::setEditorData (editor, index);
			}
			
			void FilesViewDelegate::setModelData (QWidget *editor, QAbstractItemModel *model,
					const QModelIndex& index) const
			{
				if (index.column () == 2)
				{
					int value = qobject_cast<QSpinBox*> (editor)->value ();
					model->setData (index, value);
				}
				else if (index.column () == 0)
				{
					model->setData (index,
							qobject_cast<QLineEdit*> (editor)->text ());
				}
				else
					QItemDelegate::setModelData (editor, model, index);
			}
			
		};
	};
};

