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

#include "tagsviewer.h"
#include <QInputDialog>
#include "tagsmanager.h"

using namespace LeechCraft;

TagsViewer::TagsViewer (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.TagsView_->setModel (TagsManager::Instance ().GetModel ());
}

void LeechCraft::TagsViewer::on_Rename__released ()
{
	QModelIndex index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	QString original = TagsManager::Instance ()
		.data (index, Qt::DisplayRole).toString ();

	bool ok = false;

	QString newTag = QInputDialog::getText (this,
			tr ("Rename"),
			tr ("Enter new tag name"),
			QLineEdit::Normal,
			original,
			&ok);

	if (!ok)
		return;

	TagsManager::Instance ().SetTag (index, newTag);
}

void TagsViewer::on_Remove__released ()
{
	QModelIndex index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	TagsManager::Instance ().RemoveTag (index);
}

