/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importopmldialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/either.h>
#include "common.h"
#include "exportutils.h"
#include "opmlparser.h"

namespace LC::Aggregator
{
	ImportOPMLDialog::ImportOPMLDialog (const QString& file, QWidget *parent)
	: QDialog { parent }
	, Model_ {
		Util::Field<&Item::Title_> (tr ("Title")),
		Util::Field<&Item::URL_> (tr ("URL")),
	}
	{
		Ui_.setupUi (this);
		setWindowIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		ManageLastPath ({ *Ui_.File_, "OPMLImportLastPath", {}, *this });

		Ui_.FeedsView_->setModel (&Model_);

		Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);

		Fields_ =
		{
			{ "title"_ql, Ui_.Title_ },
			{ "dateCreated"_ql, Ui_.Created_ },
			{ "dateModified"_ql, Ui_.Edited_ },
			{ "ownerName"_ql, Ui_.Owner_ },
			{ "ownerEmail"_ql, Ui_.OwnerEmail_ },
		};

		connect (Ui_.Browse_,
				&QPushButton::released,
				this,
				&ImportOPMLDialog::BrowseFile);
		connect (Ui_.File_,
				&QLineEdit::textEdited,
				this,
				&ImportOPMLDialog::HandleFilePathEdited);

		if (file.isEmpty ())
			BrowseFile ();
		else
		{
			Ui_.File_->setText (file);
			HandleFilePathEdited (file);
		}
	}

	ImportOPMLDialog::~ImportOPMLDialog () = default;

	QString ImportOPMLDialog::GetFilename () const
	{
		return Ui_.File_->text ();
	}

	QString ImportOPMLDialog::GetTags () const
	{
		return Ui_.AdditionalTags_->text ().trimmed ();
	}

	QSet<QString> ImportOPMLDialog::GetSelectedUrls () const
	{
		QSet<QString> result;
		for (const auto& item : Model_.GetItems ())
			if (item.IsChecked_)
				result << item.URL_;
		return result;
	}

	void ImportOPMLDialog::HandleFilePathEdited (const QString& newFilename)
	{
		if (QFile::exists (newFilename))
			HandleFile (newFilename);
		else
			Reset ();
	}

	void ImportOPMLDialog::BrowseFile ()
	{
		auto startingPath = Ui_.File_->text ();
		if (startingPath.isEmpty ())
			startingPath = QDir::homePath ();

		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Select OPML file"),
				startingPath,
				Util::MakeFileDialogFilter ({ { tr ("OPML files"), "opml"_ql }, { tr ("All files"), "*"_ql } }));
		if (filename.isEmpty ())
			return;

		Reset ();

		Ui_.File_->setText (filename);

		HandleFile (filename);
	}

	void ImportOPMLDialog::HandleFile (const QString& filename)
	{
		Util::Visit (ParseOPML (filename),
				[this] (const QString& error)
				{
					QMessageBox::critical (this, MessageBoxTitle, error);
					Reset ();
				},
				[this] (const OPMLParseResult& result)
				{
					for (const auto& [name, value] : result.Info_.asKeyValueRange ())
					{
						if (const auto field = Fields_.value (name))
							field->setText (value);
						else
						{
							new QTreeWidgetItem (Ui_.OtherFields_, { name, value });
							Ui_.RootLayout_->setRowVisible (Ui_.OtherFields_, true);
						}
					}

					Model_.SetItems (result.Items_);
					Ui_.FeedsView_->header ()->resizeSections (QHeaderView::ResizeToContents);
					Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (true);
				});
	}

	void ImportOPMLDialog::Reset ()
	{
		for (auto field : Fields_)
			field->clear ();

		Ui_.OtherFields_->clear ();
		Ui_.RootLayout_->setRowVisible (Ui_.OtherFields_, false);

		Model_.SetItems ({});

		Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);
	}
}
