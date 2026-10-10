/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importopmldialog.h"
#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/util.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
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

	ImportOPMLDialog::ImportOPMLDialog (const Source& source, QWidget *parent)
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

		Util::Visit (source,
				[this] (Browse)
				{
					ManageLastPath ({ *Ui_.File_, "OPMLImportLastPath", {}, *this });
					BrowseFile ();
				},
				[this] (const FilePath& path)
				{
					ManageLastPath ({ *Ui_.File_, "OPMLImportLastPath", {}, *this });
					Ui_.File_->setText (path.File_);
					HandleFilePathEdited (path.File_);
				},
				[this] (OpmlContents contents)
				{
					Ui_.File_->setText (contents.SourceName_);
					Ui_.File_->setEnabled (false);
					Ui_.Browse_->setEnabled (false);
					QBuffer buffer { &contents.Contents_ };
					HandleDevice (buffer, contents.SourceName_);
				});
	}

	ImportOPMLDialog::~ImportOPMLDialog () = default;

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
		{
			QFile file { newFilename };
			HandleDevice (file, newFilename);
		}
		else
			Reset ();
	}

	void ImportOPMLDialog::BrowseFile ()
	{
		auto startingPath = Ui_.File_->text ();
		if (startingPath.isEmpty ())
			startingPath = QDir::homePath ();

		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Import from OPML"),
				startingPath,
				Util::MakeFileDialogFilter ({ { tr ("OPML files"), "opml"_ql }, { tr ("All files"), "*"_ql } }));
		if (filename.isEmpty ())
			return;

		Reset ();

		Ui_.File_->setText (filename);

		QFile file { filename };
		HandleDevice (file, filename);
	}

	void ImportOPMLDialog::HandleDevice (QIODevice& device, const QString& context)
	{
		Util::Visit (ParseOPML (device, context),
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
