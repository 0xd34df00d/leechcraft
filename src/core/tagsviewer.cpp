/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsviewer.h"
#include <QInputDialog>
#include "tagsmanager.h"

using namespace LC;

TagsViewer::TagsViewer (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.TagsView_->setModel (TagsManager::Instance ().GetModel ());
}

void LC::TagsViewer::on_Rename__released ()
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

