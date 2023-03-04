/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importopml.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDomDocument>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include "opmlparser.h"
#include "common.h"

namespace LC
{
namespace Aggregator
{
	namespace
	{
		constexpr auto ItemUrlRole = Qt::UserRole;
	}

	ImportOPML::ImportOPML (const QString& file, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		setWindowIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());
		Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);

		if (file.isEmpty ())
			on_Browse__released ();
		else
		{
			Ui_.File_->setText (file);
			on_File__textEdited (file);
		}
	}

	QString ImportOPML::GetFilename () const
	{
		return Ui_.File_->text ();
	}

	QString ImportOPML::GetTags () const
	{
		return Ui_.AdditionalTags_->text ().trimmed ();
	}

	QSet<QString> ImportOPML::GetSelectedUrls () const
	{
		QSet<QString> result;

		for (int i = 0, items = Ui_.FeedsToImport_->topLevelItemCount (); i < items; ++i)
		{
			const auto item = Ui_.FeedsToImport_->topLevelItem (i);
			if (item->data (0, Qt::CheckStateRole) == Qt::Checked)
				result << item->data (0, ItemUrlRole).toString ();
		}

		return result;
	}

	void ImportOPML::on_File__textEdited (const QString& newFilename)
	{
		if (QFile::exists (newFilename))
			HandleFile (newFilename);
		else
			Reset ();
	}

	void ImportOPML::on_Browse__released ()
	{
		auto startingPath = QFileInfo (Ui_.File_->text ()).path ();
		if (startingPath.isEmpty ())
			startingPath = QDir::homePath ();

		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Select OPML file"),
				startingPath,
				tr ("OPML files (*.opml);;"
					"XML files (*.xml);;"
					"All files (*.*)"));
		if (filename.isEmpty ())
			return;

		Reset ();

		Ui_.File_->setText (filename);

		HandleFile (filename);
	}

	void ImportOPML::HandleFile (const QString& filename)
	{
		Util::Visit (ParseOPML (filename),
				[this] (const QString& error)
				{
					QMessageBox::critical (this, MessageBoxTitle, error);
					Reset ();
				},
				[this] (OPMLParser parser)
				{
					for (const auto& [name, value] : Util::Stlize (parser.GetInfo ()))
					{
						if (name == "title")
							Ui_.Title_->setText (value);
						else if (name == "dateCreated")
							Ui_.Created_->setText (value);
						else if (name == "dateModified")
							Ui_.Edited_->setText (value);
						else if (name == "ownerName")
							Ui_.Owner_->setText (value);
						else if (name == "ownerEmail")
							Ui_.OwnerEmail_->setText (value);
						else
							new QTreeWidgetItem (Ui_.OtherFields_, { name, value });
					}

					for (const auto& opmlItem : parser.Parse ())
					{
						const auto item = new QTreeWidgetItem (Ui_.FeedsToImport_, { opmlItem.Title_, opmlItem.URL_ });
						item->setData (0, Qt::CheckStateRole, Qt::Checked);
						item->setData (0, ItemUrlRole, opmlItem.URL_);
					}

					Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (true);
				});
	}

	void ImportOPML::Reset ()
	{
		Ui_.Title_->setText ("");
		Ui_.Created_->setText ("");
		Ui_.Edited_->setText ("");
		Ui_.Owner_->setText ("");
		Ui_.OwnerEmail_->setText ("");
		Ui_.OtherFields_->clear ();
		Ui_.FeedsToImport_->clear ();

		Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);
	}
}
}
