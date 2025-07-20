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
#include <util/tags/tagseditdelegate.h>
#include "common.h"
#include "exportutils.h"
#include "opmlparser.h"

namespace LC::Aggregator
{
	enum Columns
	{
		Title,
		Categories,
		URL,
	};

	ImportOPMLDialog::ImportOPMLDialog (const QString& file, QWidget *parent)
	: QDialog { parent }
	, Model_ {
		std::tuple { Util::ItemsEditable::Param { { 1, 2 } } },
		Util::Field<&Item::Title_> (tr ("Title")),
		Util::Field<&Item::Categories_> (tr ("Categories")),
		Util::Field<&Item::URL_> (tr ("URL")),
	}
	{
		Ui_.setupUi (this);
		setWindowIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		ManageLastPath ({ *Ui_.File_, "OPMLImportLastPath", {}, *this });

		Ui_.FeedsView_->setModel (&Model_);
		Ui_.FeedsView_->setItemDelegateForColumn (Categories,
				new Util::TagsEditDelegate { *GetProxyHolder ()->GetTagsManager (), this });

		Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);

		Fields_ =
		{
			{ u"title"_qsv, Ui_.Title_ },
			{ u"dateCreated"_qsv, Ui_.Created_ },
			{ u"dateModified"_qsv, Ui_.Edited_ },
			{ u"ownerName"_qsv, Ui_.Owner_ },
			{ u"ownerEmail"_qsv, Ui_.OwnerEmail_ },
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

	QList<OPMLItem> ImportOPMLDialog::GetSelectedItems () const
	{
		QList<OPMLItem> result;
		for (const auto& item : Model_.GetItems ())
			if (item.IsChecked_)
				result << item;
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
