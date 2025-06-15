/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filesviewdelegate.h"
#include <QModelIndex>
#include <QSpinBox>
#include <QLineEdit>
#include <QApplication>
#include <QTreeView>
#include <util/util.h>
#include <util/gui/util.h>
#include "torrentfilesmodel.h"

namespace LC::BitTorrent
{
	FilesViewDelegate::FilesViewDelegate (QTreeView *parent)
	: QStyledItemDelegate { parent }
	, View_ { parent }
	{
	}

	QWidget* FilesViewDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case TorrentFilesModel::ColumnPriority:
		{
			const auto box = new QSpinBox (parent);
			box->setRange (0, 7);
			return box;
		}
		case TorrentFilesModel::ColumnPath:
			return new QLineEdit { parent };
		default:
			return QStyledItemDelegate::createEditor (parent, option, index);
		}
	}

	void FilesViewDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case TorrentFilesModel::ColumnPriority:
			qobject_cast<QSpinBox*> (editor)->setValue (index.data (TorrentFilesModel::RolePriority).toInt ());
			break;
		case TorrentFilesModel::ColumnPath:
			qobject_cast<QLineEdit*> (editor)->setText (index.data (TorrentFilesModel::RoleFullPath).toString ());
			break;
		default:
			QStyledItemDelegate::setEditorData (editor, index);
			break;
		}
	}

	void FilesViewDelegate::setModelData (QWidget *editor, QAbstractItemModel *model,
			const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case TorrentFilesModel::ColumnPriority:
		{
			const int value = qobject_cast<QSpinBox*> (editor)->value ();
			const auto& sindexes = View_->selectionModel ()->selectedRows ();
			for (const auto& selected : sindexes)
				model->setData (index.sibling (selected.row (), index.column ()), value);
			break;
		}
		case TorrentFilesModel::ColumnPath:
		{
			const auto& oldData = index.data (TorrentFilesModel::RoleFullPath);
			const auto& newText = qobject_cast<QLineEdit*> (editor)->text ();
			if (oldData.toString () == newText)
				return;

			model->setData (index, newText);
			break;
		}
		default:
			QStyledItemDelegate::setModelData (editor, model, index);
			break;
		}
	}

	void FilesViewDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.column () != TorrentFilesModel::ColumnPath)
		{
			QStyledItemDelegate::updateEditorGeometry (editor, option, index);
			return;
		}

		auto rect = option.rect;
		rect.setX (0);
		rect.setWidth (editor->parentWidget ()->width ());
		editor->setGeometry (rect);
	}
}
