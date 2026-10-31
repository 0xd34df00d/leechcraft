/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsviewer.h"
#include <QInputDialog>
#include <QMessageBox>
#include <util/sll/visitor.h>
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
	const auto& index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	const auto& original = index.data ().toString ();

	bool ok = false;
	const auto& newTag = QInputDialog::getText (this,
			tr ("Rename"),
			tr ("Enter new tag name"),
			QLineEdit::Normal,
			original,
			&ok).trimmed ();
	if (!ok || newTag == original)
		return;

	auto& tm = TagsManager::Instance ();
	if (tm.GetAllTags ().contains (newTag) &&
			QMessageBox::question (this,
					tr ("Rename"),
					tr ("There is already a tag named %1. Merge into it?").arg (newTag)) != QMessageBox::Yes)
		return;

	const auto& result = tm.ReplaceName (original, { newTag });
	if (result.IsLeft ())
		QMessageBox::warning (this,
				tr ("Rename"),
				Util::Visit (result.GetLeft (),
						[] (TagsManager::BlankTagName) { return tr ("The tag name cannot be empty."); },
						[] (const TagsManager::DelimiterInTagName& delimited) { return tr ("The tag name cannot contain \"%1\".").arg (delimited.Delimiter_); }));
}

void TagsViewer::on_Remove__released ()
{
	const auto& index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	TagsManager::Instance ().ReplaceName (index.data ().toString (), {});
}

