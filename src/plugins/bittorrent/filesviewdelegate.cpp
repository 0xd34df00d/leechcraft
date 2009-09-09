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
#include <QApplication>
#include <QTreeView>
#include <QtDebug>
#include <plugininterface/proxy.h>
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
			
			FilesViewDelegate::FilesViewDelegate (QTreeView *parent)
			: QStyledItemDelegate (parent)
			, View_ (parent)
			{
			}
			
			FilesViewDelegate::~FilesViewDelegate ()
			{
			}
			
			QWidget* FilesViewDelegate::createEditor (QWidget *parent,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				if (index.column () == TorrentFilesModel::ColumnPriority &&
						!HasChildren (index))
				{
					QSpinBox *box = new QSpinBox (parent);
					box->setRange (0, 7);
					return box;
				}
				else if (index.column () == TorrentFilesModel::ColumnPath &&
						!HasChildren (index))
					return new QLineEdit (parent);
				else
					return QStyledItemDelegate::createEditor (parent, option, index);
			}
			
			void FilesViewDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				if (index.column () != TorrentFilesModel::ColumnProgress)
				{
					QStyledItemDelegate::paint (painter, option, index);
					return;
				}
                else
				{
					QStyleOptionProgressBar progressBarOption;
					progressBarOption.state = QStyle::State_Enabled;
					progressBarOption.direction = QApplication::layoutDirection ();
					progressBarOption.rect = option.rect;
					progressBarOption.fontMetrics = QApplication::fontMetrics ();
					progressBarOption.minimum = 0;
					progressBarOption.maximum = 100;
					progressBarOption.textAlignment = Qt::AlignCenter;
					progressBarOption.textVisible = true;

					double progress = index.data (TorrentFilesModel::RoleProgress).toDouble ();
					int size = index.data (TorrentFilesModel::RoleSize).toInt ();
					int done = progress * size;
					progressBarOption.progress = progress < 0 ? 0 : progress * 100;
					progressBarOption.text = QString (tr ("%1% (%2 of %3)")
							.arg (static_cast<int> (progress * 100))
							.arg (Util::Proxy::Instance ()->MakePrettySize (done))
							.arg (Util::Proxy::Instance ()->MakePrettySize (size)));

					QApplication::style ()->drawControl (QStyle::CE_ProgressBar,
							&progressBarOption, painter);
				}
			}
			
			void FilesViewDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
			{
				if (index.column () == TorrentFilesModel::ColumnPriority &&
						!HasChildren (index))
					qobject_cast<QSpinBox*> (editor)->
						setValue (static_cast<TreeItem*> (index.internalPointer ())->
								Data (1).toInt ());
				else if (index.column () == TorrentFilesModel::ColumnPath &&
						!HasChildren (index))
				{
					QVariant data = static_cast<TreeItem*> (index.internalPointer ())->
						Data (0, TorrentFilesModel::RawDataRole);
					qobject_cast<QLineEdit*> (editor)->setText (data.toString ());
				}
				else
					QStyledItemDelegate::setEditorData (editor, index);
			}
			
			void FilesViewDelegate::setModelData (QWidget *editor, QAbstractItemModel *model,
					const QModelIndex& index) const
			{
				if (index.column () == TorrentFilesModel::ColumnPriority)
				{
					int value = qobject_cast<QSpinBox*> (editor)->value ();
					QModelIndexList sindexes = View_->selectionModel ()->selectedRows ();
					Q_FOREACH (QModelIndex selected, sindexes)
						model->setData (index.sibling (selected.row (), index.column ()), value);
				}
				else if (index.column () == TorrentFilesModel::ColumnPath)
				{
					QVariant oldData = static_cast<TreeItem*> (index.internalPointer ())->
						Data (0, TorrentFilesModel::RawDataRole);
					QString newText = qobject_cast<QLineEdit*> (editor)->text ();
					if (oldData.toString () == newText)
						return;

					model->setData (index, newText);
				}
				else
					QStyledItemDelegate::setModelData (editor, model, index);
			}
			
		};
	};
};

