/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsviewer.h"
#include <optional>
#include <QAbstractItemView>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <util/gui/util.h>
#include <util/sll/visitor.h>
#include <util/tags/tagseditoraddon.h>
#include "tagsmanager.h"

using namespace LC;

TagsViewer::TagsViewer (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
	Ui_.TagsView_->setModel (TagsManager::Instance ().GetModel ());
}

namespace
{
	std::optional<QStringList> AskNewNames (QWidget *parent, const QString& original)
	{
		QDialog dialog { parent };
		dialog.setWindowTitle (TagsViewer::tr ("Modify"));

		const auto edit = new QLineEdit;
		Util::TagsEditorAddon addon { *edit, *TagsManager::Instance ().GetModel () };
		addon.SetTags ({ original });

		const auto buttons = new QDialogButtonBox { QDialogButtonBox::Ok | QDialogButtonBox::Cancel };
		QObject::connect (buttons,
				&QDialogButtonBox::accepted,
				&dialog,
				&QDialog::accept);
		QObject::connect (buttons,
				&QDialogButtonBox::rejected,
				&dialog,
				&QDialog::reject);

		const auto layout = new QVBoxLayout { &dialog };
		layout->addWidget (new QLabel { TagsViewer::tr ("Enter the new name of %1, or several names to split it:").arg (original) });
		layout->addWidget (edit);
		layout->addWidget (buttons);

		if (dialog.exec () != QDialog::Accepted)
			return {};
		return addon.GetTags ();
	}

	QString Describe (const TagsManager::InvalidTagName& error)
	{
		return Util::Visit (error,
				[] (TagsManager::BlankTagName) { return TagsViewer::tr ("The tag name cannot be empty."); },
				[] (const TagsManager::DelimiterInTagName& delimited) { return TagsViewer::tr ("The tag name cannot contain \"%1\".").arg (delimited.Delimiter_); });
	}

	void SelectTag (QAbstractItemView& view, const QString& name)
	{
		const auto model = view.model ();
		const auto& matches = model->match (model->index (0, 0), Qt::DisplayRole, name, 1, Qt::MatchExactly | Qt::MatchCaseSensitive);
		if (!matches.isEmpty ())
			view.setCurrentIndex (matches.first ());
	}
}

void LC::TagsViewer::on_Modify__released ()
{
	const auto& index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	const auto& original = index.data ().toString ();
	const auto& names = AskNewNames (this, original);
	if (!names || *names == QStringList { original })
		return;

	if (names->isEmpty ())
	{
		QMessageBox::warning (this, tr ("Modify"), Describe (TagsManager::BlankTagName {}));
		return;
	}

	auto& tm = TagsManager::Instance ();
	const auto& allTags = tm.GetAllTags ();
	QStringList existing;
	for (const auto& name : *names)
		if (name != original && allTags.contains (name))
			existing << name;
	if (!existing.isEmpty () &&
			QMessageBox::question (this,
					tr ("Modify"),
					Util::FormatHumanReadableList (existing,
							tr ("There is already a tag named %1. Merge into it?"),
							tr ("There are already tags named %1. Merge into them?"),
							tr ("There are already %n tags with these names. Merge into them?", nullptr, existing.size ()))) != QMessageBox::Yes)
		return;

	const auto& result = tm.ReplaceName (original, *names);
	if (result.IsLeft ())
	{
		QMessageBox::warning (this, tr ("Modify"), Describe (result.GetLeft ()));
		return;
	}

	SelectTag (*Ui_.TagsView_, names->first ());
}

void TagsViewer::on_Remove__released ()
{
	const auto& index = Ui_.TagsView_->currentIndex ();
	if (!index.isValid ())
		return;

	const auto& name = index.data ().toString ();
	if (QMessageBox::question (this,
				tr ("Remove"),
				tr ("Remove the tag %1? Items tagged with it will lose it.").arg (name)) != QMessageBox::Yes)
		return;

	TagsManager::Instance ().ReplaceName (name, {});
}
