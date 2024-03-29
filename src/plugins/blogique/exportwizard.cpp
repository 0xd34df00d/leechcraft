/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin  <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "exportwizard.h"
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QPrinter>
#include <QCloseEvent>
#include <QDomElement>
#include <QDomText>
#include <QButtonGroup>
#include <QTextDocument>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include "interfaces/blogique/ibloggingplatform.h"
#include "core.h"

namespace LC
{
namespace Blogique
{
	ExportWizard::ExportWizard(QWidget *parent)
	: QWizard (parent)
	, AllTagsModel_ (new QStandardItemModel (this))
	, SelectedTagsModel_ (new QStandardItemModel (this))
	, Formats_ (new QButtonGroup (this))
	{
		Ui_.setupUi (this);

		connect (this,
				SIGNAL (currentIdChanged (int)),
				this,
				SLOT (handleCurrentIdChanged (int)));

		connect (Ui_.AccountSelection_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleAccountChanged (int)));

		for (auto acc : Core::Instance ().GetAccounts ())
		{
			if (auto bp = qobject_cast<IBloggingPlatform*> (acc->GetParentBloggingPlatform ()))
			{
				Ui_.AccountSelection_->addItem (bp->GetBloggingPlatformIcon (),
						acc->GetAccountName ());
				Id2Account_ [Ui_.AccountSelection_->count () - 1] = acc;

				connect (acc->GetQObject (),
						SIGNAL (tagsUpdated (QHash<QString, int>)),
						this,
						SLOT (handleTagsUpdated (QHash<QString, int>)));
				acc->RequestTags ();
			}
		}

		int i = 0;
		for (auto btn : Ui_.OutputFormatPage_->findChildren<QRadioButton*> ())
		{
			Id2RadioButton_ [i] = btn;
			Formats_->addButton (btn, i++);
		}
		Ui_.PlainText_->setProperty ("ExportFormat", ExportFormat::PlainText);
		Ui_.Html_->setProperty ("ExportFormat", ExportFormat::Html);
		Ui_.Fb2_->setProperty ("ExportFormat", ExportFormat::Fb2);
		Ui_.Pdf_->setProperty ("ExportFormat", ExportFormat::Pdf);

		Ui_.FromDate_->setCalendarPopup (true);
		Ui_.TillDate_->setCalendarPopup (true);
		Ui_.TillDate_->setDateTime (QDateTime::currentDateTime ());

		Ui_.AllTagsView_->setModel (AllTagsModel_);
		Ui_.AllTagsView_->setHeaderHidden (true);
		Ui_.SelectedTagsView_->setModel (SelectedTagsModel_);
		Ui_.SelectedTagsView_->setHeaderHidden (true);

		Ui_.WelcomePageErrorLabel_->setVisible (false);
		Ui_.ExportPageErrorLabel_->setVisible (false);
		Ui_.ContentPageDateErrorLabel_->setVisible (false);
		Ui_.ContentPageTagsErrorLabel_->setVisible (false);

		connect (Ui_.AddTag_,
				SIGNAL(released ()),
				this,
				SLOT (addTag ()));
		connect (Ui_.RemoveTag_,
				SIGNAL (released ()),
				this,
				SLOT (removeTag ()));

		connect (Ui_.SelectPath_,
				SIGNAL (released ()),
				this,
				SLOT (selectExportPath ()));
	}

	void ExportWizard::FillTags (IAccount *acc)
	{
		if (auto rc = AllTagsModel_->rowCount ())
			AllTagsModel_->removeRows (0, rc);

		for (const auto& tag : Account2Tags_.value (acc))
		{
			auto item = new QStandardItem (tag);
			item->setEditable (false);
			AllTagsModel_->appendRow (item);
		}
	}

	bool ExportWizard::validateCurrentPage ()
	{
		switch (currentId ())
		{
		case WelcomePage:
			if ( Ui_.AccountSelection_->currentIndex () == -1)
			{
				Ui_.WelcomePageErrorLabel_->setVisible (true);
				return false;
			}
			return true;
		case FormatPage:
			if (Ui_.SavePath_->text ().isEmpty ())
			{
				Ui_.ExportPageErrorLabel_->setVisible (true);
				return false;
			}
			return true;
		case ContentPage:
			if (Ui_.WithDateRange_->isChecked () &&
					(Ui_.FromDate_->dateTime () > Ui_.TillDate_->dateTime ()))
			{
				Ui_.ContentPageDateErrorLabel_->setVisible (true);
				return false;
			}

			if (Ui_.SelectedTags_->isChecked () &&
					!SelectedTagsModel_->rowCount ())
			{
				Ui_.ContentPageTagsErrorLabel_->setVisible (true);
				return false;
			}
			return true;
		case OverviewPage:
		case ExportPage:
			Ui_.ExportProgress_->setMinimum (0);
			Ui_.ExportProgress_->setMaximum (0);
			return true;
		}

		return true;
	}

	void ExportWizard::reject ()
	{
		deleteLater ();
		QDialog::reject ();
	}

	void ExportWizard::handleAccountChanged (int index)
	{
		if (index == -1)
			return;

		FillTags (Id2Account_.value (index));
	}

	void ExportWizard::handleCurrentIdChanged (int id)
	{
		switch (id)
		{
			case WelcomePage:
			case FormatPage:
			case ContentPage:
				Ui_.FromDate_->setDate (QDate::fromString ("01.01.1970", "dd.MM.yyyy"));
				Ui_.TillDate_->setDate (QDate::currentDate ());
				break;
			case OverviewPage:
			{
				Ui_.AccountLabel_->setText (Ui_.AccountSelection_->currentText ());
				Ui_.FormatLabel_->setText (Formats_->checkedButton ()->text ());
				Ui_.EntriesLabel_->setText (Ui_.AllEntries_->isChecked () ?
					tr ("All entries") :
					tr ("Only between %1 and %2")
							.arg (Ui_.FromDate_->text ())
							.arg (Ui_.TillDate_->text ()));
				QStringList selectedTags;
				for (int i = 0; Ui_.SelectedTags_->isChecked () && i < SelectedTagsModel_->rowCount ();
						++i)
					 selectedTags << SelectedTagsModel_->index (i, 0).data ().toString ();
				Ui_.EntriesTagsLabel_->setText (Ui_.AllTags_->isChecked () ?
							tr ("All tags") :
							tr ("Only tags: %1")
						.arg (selectedTags.join (", ")));
				Ui_.SavePathLabel_->setText (Ui_.SavePath_->text ());
				break;
			}
			case ExportPage:
			{
				if (!Id2Account_.contains (Ui_.AccountSelection_->currentIndex ()))
					return;

				Filter filter;
				filter.CustomDate_ = Ui_.WithDateRange_->isChecked ();
				filter.BeginDate_ = Ui_.FromDate_->dateTime ();
				filter.EndDate_ = Ui_.TillDate_->dateTime ();
				QStringList selectedTags;
				for (int i = 0; Ui_.SelectedTags_->isChecked () && i < SelectedTagsModel_->rowCount ();
						++i)
					selectedTags << SelectedTagsModel_->index (i, 0).data ().toString ();
				filter.Tags_ = Ui_.SelectedTags_->isChecked () ? selectedTags : QStringList ();

				auto account = Id2Account_ [Ui_.AccountSelection_->currentIndex ()];
				connect (account->GetQObject (),
						SIGNAL (gotFilteredEntries (QList<Entry>)),
						this,
						SLOT (handleGotFilteredEntries (QList<Entry>)));
				connect (account->GetQObject (),
						SIGNAL (gettingFilteredEntriesFinished ()),
						this,
						SLOT (handleGettingFilteredEntriesFinished ()));
				account->GetEntriesWithFilter (filter);
				break;
			}
			default:
				break;
		}
	}

	void ExportWizard::selectExportPath ()
	{
		const auto& path = QFileDialog::getSaveFileName (this,
				tr ("Select export path"),
				QDir::homePath ());

		Ui_.SavePath_->setText (path);
	}

	void ExportWizard::addTag ()
	{
		auto srcIndex = Ui_.AllTagsView_->selectionModel ()->selectedRows ().value (0);
		if (!srcIndex.isValid ())
			return;

		auto row = AllTagsModel_->takeRow (srcIndex.row ());
		SelectedTagsModel_->appendRow (row);
		if (Ui_.ContentPageTagsErrorLabel_->isVisible ())
			Ui_.ContentPageTagsErrorLabel_->setVisible (false);
	}

	void ExportWizard::removeTag ()
	{
		auto srcIndex = Ui_.SelectedTagsView_->selectionModel ()->selectedRows ().value (0);
		if (!srcIndex.isValid ())
			return;

		auto row = SelectedTagsModel_->takeRow (srcIndex.row ());
		AllTagsModel_->appendRow (row);
	}

	void ExportWizard::on_AccountSelection__currentIndexChanged (int index)
	{
		if (Ui_.WelcomePageErrorLabel_->isVisible () && index != -1)
			Ui_.WelcomePageErrorLabel_->setVisible (false);
	}

	void ExportWizard::on_SavePath__textChanged (const QString& text)
	{
		if (Ui_.ExportPageErrorLabel_->isVisible () && !text.isEmpty ())
			Ui_.ExportPageErrorLabel_->setVisible (false);
	}

	void ExportWizard::on_FromDate__dateChanged (const QDate& date)
	{
		if (Ui_.ContentPageDateErrorLabel_->isVisible () &&
				date < Ui_.TillDate_->date ())
			Ui_.ContentPageDateErrorLabel_->setVisible (false);
	}

	void ExportWizard::on_TillDate__dateChanged (const QDate& date)
	{
		if (Ui_.ContentPageDateErrorLabel_->isVisible () &&
				date > Ui_.FromDate_->date ())
			Ui_.ContentPageDateErrorLabel_->setVisible (false);
	}

	void ExportWizard::handleTagsUpdated (const QHash<QString, int>& tags)
	{
		if (auto acc = qobject_cast<IAccount*> (sender ()))
		{
			const auto& tagsNames = tags.keys ();
			Account2Tags_ [acc] = tagsNames;

			if (acc != Id2Account_ [Ui_.AccountSelection_->currentIndex ()])
				return;

			FillTags (acc);
		}
	}

	namespace
	{
		QString GetHtmlContent (const QList<Entry>& entries)
		{
			QString content;
			QLocale loc;
			for (const auto& entry : entries)
			{
				content += "<br/><br/><br/><br/><em>" + loc.toString (entry.Date_, QLocale::LongFormat) + "</em><br/><br/>";
				content += "<strong>" + entry.Subject_ + "</strong><br/><br/>";
				content += entry.Content_ + "<br/><br/>";
				content += ("<strong>Tags:</strong><em>" + entry.Tags_.join (",") + "</em><br/><br/><br/>");
			}

			return content;
		}

		QString GetPlainTextContent (const QList<Entry>& entries)
		{
			QString content;
			QLocale loc;
			for (const auto& entry : entries)
			{
				content += "\n\n\n\n" + loc.toString (entry.Date_, QLocale::LongFormat) + "\n\n";
				content += entry.Subject_ + "\n\n\n";
				content += entry.Content_ + "\n\n";
				content += "Tags: " + entry.Tags_.join (",") + "\n\n\n";
			}

			return content;
		}


		void WritePlainText (const QList<Entry>& entries, const QString& filePath)
		{
			QFile file (filePath);
			if (!file.open (QIODevice::WriteOnly))
			{
				QMessageBox::warning (nullptr,
						"LeechCraft",
						QObject::tr ("Unable to open file %1: %2")
								.arg (filePath)
								.arg (file.errorString ()));
				return;
			}

			QTextDocument doc;
			doc.setHtml (GetPlainTextContent (entries));
			file.write (doc.toPlainText ().toUtf8 ());
		}

		void WriteHtml (const QList<Entry>& entries, const QString& filePath)
		{
			QFile file (filePath);
			if (!file.open (QIODevice::WriteOnly))
			{
				QMessageBox::warning (nullptr,
						"LeechCraft",
						QObject::tr ("Unable to open file %1: %2")
								.arg (filePath)
								.arg (file.errorString ()));
				return;
			}

			file.write (GetHtmlContent (entries).toUtf8 ());
		}

		void WriteFb2 (const QList<Entry>& entries, const QString& filePath)
		{
			QDomDocument doc;
			QDomElement root = doc.createElement ("FictionBook");
			root.setAttribute ("xmlns", "http://www.gribuser.ru/xml/fictionbook/2.0");
			root.setAttribute ("xmlns:l", "http://www.w3.org/1999/xlink");
			doc.appendChild (root);
			QDomProcessingInstruction instruction = doc.createProcessingInstruction ("xml", "version=\"1.0\" encoding=\"UTF-8\"");
			doc.insertBefore (instruction, root);
			QDomElement description = doc.createElement ("description");
			root.appendChild (description);
			QDomElement titleInfo = doc.createElement ("title-info");
			description.appendChild (titleInfo);
			QDomElement bookTitle = doc.createElement ("book-title");
			titleInfo.appendChild (bookTitle);
			QDomText bookTitleText = doc.createTextNode ("Exported blog");
			bookTitle.appendChild (bookTitleText);
			QDomElement body = doc.createElement ("body");
			root.appendChild (body);

			for (const auto& entry : entries)
			{
				QDomElement section = doc.createElement ("section");
				body.appendChild (section);
				QDomElement sectionTitle = doc.createElement ("title");
				section.appendChild (sectionTitle);
				QDomElement p1 = doc.createElement ("p");
				sectionTitle.appendChild (p1);
				QDomElement strong = doc.createElement ("strong");
				p1.appendChild (strong);
				QDomText sectionTitleSubject = doc.createTextNode (entry.Subject_);
				strong.appendChild (sectionTitleSubject);
				QDomElement p2 = doc.createElement ("p");
				section.appendChild (p2);
				QDomElement emphasis = doc.createElement ("emphasis");
				p2.appendChild (emphasis);
				QDomText sectionTitleDate = doc.createTextNode (QLocale {}.toString (entry.Date_, QLocale::LongFormat));
				emphasis.appendChild (sectionTitleDate);

				QTextDocument converter;
				converter.setHtml (entry.Content_.toUtf8 ());

				QDomElement p3 = doc.createElement ("p");
				section.appendChild (p3);
				QDomText content = doc.createTextNode (converter.toPlainText ());
				p3.appendChild (content);
			}

			QFile file (filePath);
			if (!file.open (QIODevice::WriteOnly))
			{
				QMessageBox::warning (0,
					"LeechCraft",
					QObject::tr ("Unable to open file %1: %2")
						.arg (filePath)
						.arg (file.errorString ()));
				return;
			}

			file.write (doc.toByteArray ());
		}

		void WritePdf (const QList<Entry>& entries, const QString& filePath)
		{
			// TODO use poshuku to print to pdf
			QTextDocument doc;
			doc.setHtml (GetHtmlContent (entries));

			QPrinter printer (QPrinter::HighResolution);
			printer.setPageSize (QPageSize { QPageSize::A4 });
			printer.setOutputFormat (QPrinter::PdfFormat);

			printer.setOutputFileName (filePath);

			doc.print (&printer);
		}
	}

	void ExportWizard::handleGotFilteredEntries (const QList<Entry>& entries)
	{
		Entries_ << entries;
	}

	void ExportWizard::handleGettingFilteredEntriesFinished ()
	{
		switch (Formats_->checkedButton ()->property ("ExportFormat").toInt ())
		{
			case PlainText:
				WritePlainText (Entries_, Ui_.SavePath_->text ());
				break;
			case Html:
				WriteHtml (Entries_, Ui_.SavePath_->text ());
				break;
			case Fb2:
				WriteFb2 (Entries_, Ui_.SavePath_->text ());
				break;
			case Pdf:
				WritePdf (Entries_, Ui_.SavePath_->text ());
				break;
			default:
				return;
		}

		Core::Instance ().GetCoreProxy ()->GetEntityManager ()->
				HandleEntity (Util::MakeNotification ("Blogique",
						tr ("Exporting finished"),
						Priority::Info));
		deleteLater ();
	}

}
}
