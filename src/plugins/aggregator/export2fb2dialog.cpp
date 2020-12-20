/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "export2fb2dialog.h"
#include <algorithm>
#include <QXmlStreamWriter>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>
#include <QDate>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextCursor>
#include <QPrinter>
#include <interfaces/structures.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/tags/categoryselector.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "channelsmodel.h"
#include "storagebackendmanager.h"
#include "itemutils.h"

namespace LC
{
namespace Aggregator
{
	Export2FB2Dialog::Export2FB2Dialog (ChannelsModel *channelsModel, const ICoreProxy_ptr& proxy, QWidget *parent)
	: QDialog { parent }
	, ChannelsModel_ { channelsModel }
	, Proxy_ { proxy }
	{
		Ui_.setupUi (this);

		Ui_.ChannelsTree_->setModel (channelsModel);

		Selector_ = new Util::CategorySelector (this);
		Selector_->setWindowFlags (Qt::Widget);
		Selector_->SetPossibleSelections ({});
		Ui_.VLayout_->addWidget (Selector_);

		connect (Ui_.ChannelsTree_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				this,
				&Export2FB2Dialog::handleChannelsSelectionChanged);

		for (int i = 0; i < Ui_.FB2Genres_->topLevelItemCount (); ++i)
		{
			QTreeWidgetItem *item = Ui_.FB2Genres_->topLevelItem (i);
			for (int j = 0; j < item->childCount (); ++j)
			{
				QTreeWidgetItem *subItem = item->child (j);
				if (subItem->checkState (0) == Qt::Unchecked)
					subItem->setCheckState (0, Qt::Unchecked);
			}
		}

		connect (this,
				&Export2FB2Dialog::accepted,
				this,
				&Export2FB2Dialog::handleAccepted);

		on_File__textChanged (QString ());
	}

	struct WriteInfo
	{
		const QString File_;
		const QMap<ChannelShort, QList<Item>> Items_;
	};

	namespace
	{
		QString FixContents (QString descr)
		{
			descr.replace (QRegExp ("</p>\\s*<p>"), "<br/>");
			descr.remove ("<p>");
			descr.remove ("</p>");

			// Remove images, links and frames
			QRegExp imgRx ("<img *>", Qt::CaseSensitive, QRegExp::Wildcard);
			imgRx.setMinimal (true);
			descr.remove (imgRx);
			descr.remove ("</img>");

			// Remove tables
			if (descr.contains ("<table", Qt::CaseInsensitive))
			{
				QRegExp tableRx ("<table.*/table>", Qt::CaseInsensitive);
				tableRx.setMinimal (true);
				descr.remove (tableRx);
			}

			// Objects
			if (descr.contains ("<object", Qt::CaseInsensitive))
			{
				QRegExp objRx ("<object.*/object>", Qt::CaseInsensitive);
				objRx.setMinimal (true);
				descr.remove (objRx);
			}

			QRegExp linkRx ("<a.*>");
			linkRx.setMinimal (true);
			descr.remove (linkRx);
			descr.remove ("</a>");

			QRegExp iframeRx ("<iframe .*/iframe>");
			iframeRx.setMinimal (true);
			descr.remove (iframeRx);

			// Replace HTML entities with corresponding stuff
			descr.replace ("&qout;", "\"");
			descr.replace ("&emdash;", QString::fromUtf8 ("—"));
			descr.replace ("&mdash;", QString::fromUtf8 ("—"));
			descr.replace ("&ndash;", "-");

			// Remove the rest
			descr.replace ("&amp;", "&&");
			descr.remove (QRegExp ("&\\w*;"));
			descr.replace ("&&", "&amp;");

			// Fix some common errors
			descr.replace ("<br>", "<br/>");
			descr.replace (QRegExp ("<br\\s+/>"), "<br/>");

			// Replace multilines
			while (descr.contains ("<br/><br/>"))
				descr.replace ("<br/><br/>", "<br/>");

			// Replace HTML tags with their fb2 analogues
			descr.replace ("<em>", "<emphasis>", Qt::CaseInsensitive);
			descr.replace ("</em>", "</emphasis>", Qt::CaseInsensitive);
			descr.replace ("<i>", "<emphasis>", Qt::CaseInsensitive);
			descr.replace ("</i>", "</emphasis>", Qt::CaseInsensitive);
			descr.replace ("<b>", "<strong>", Qt::CaseInsensitive);
			descr.replace ("</b>", "</strong>", Qt::CaseInsensitive);
			descr.replace ("<ss>", "<strikethrough>", Qt::CaseInsensitive);
			descr.replace ("</ss>", "</strikethrough>", Qt::CaseInsensitive);

			if (descr.endsWith ("<br/>"))
				descr.chop (5);

			// Remove unclosed tags
			QRegExp unclosedRx ("<(\\w+)[^/]*>");
			unclosedRx.setMinimal (true);
			int pos = 0;
			while ((pos = unclosedRx.indexIn (descr, pos)) != -1)
			{
				QRegExp closedFinder ("</" + unclosedRx.cap (1) + ">");
				if (closedFinder.indexIn (descr, pos + unclosedRx.matchedLength ()) != -1)
				{
					pos += unclosedRx.matchedLength ();
					continue;
				}
				descr.remove (pos, unclosedRx.matchedLength ());
			}

			// Normalize empty lines - needs to be done after removing
			// unclosed, otherwise last <p> would get dropped.
			descr.replace (QRegExp ("<br/>\\s*</p>"), "</p>");
			descr.replace ("<br/>", "</p><p>");

			descr.remove ("\r");
			descr.remove ("\n");
			descr = descr.simplified ();

			return descr;
		}

		void WriteChannel (QXmlStreamWriter& w,
				const ChannelShort& cs, const QList<Item>& items)
		{
			w.writeStartElement ("section");
				w.writeAttribute ("id", QString::number (cs.ChannelID_));
				w.writeStartElement ("title");
					w.writeTextElement ("p", FixContents (cs.Title_));
				w.writeEndElement ();
				w.writeTextElement ("annotation",
						Export2FB2Dialog::tr ("%n unread item(s)", "", cs.Unread_));
				for (const auto& item : items)
				{
					w.writeStartElement ("title");
						w.writeStartElement ("p");
						w.writeComment ("p");
						w.device ()->write (FixContents (item.Title_).toUtf8 ());
						w.writeEndElement ();
					w.writeEndElement ();

					bool hasDate = item.PubDate_.isValid ();
					bool hasAuthor = !item.Author_.isEmpty ();
					if (hasDate || hasAuthor)
					{
						w.writeStartElement ("epigraph");
							if (hasDate)
								w.writeTextElement ("p",
										Export2FB2Dialog::tr ("Published on %1")
											.arg (item.PubDate_.toString ()));
							if (hasAuthor)
								w.writeTextElement ("p",
										Export2FB2Dialog::tr ("By %1")
											.arg (item.Author_));
						w.writeEndElement ();
						w.writeEmptyElement ("empty-line");
					}

					w.writeStartElement ("p");
						w.writeComment ("p");
						w.device ()->write (FixContents (item.Description_).toUtf8 ());
					w.writeEndElement ();

					w.writeEmptyElement ("empty-line");
				}
			w.writeEndElement ();
		}

		void WriteBeginning (QXmlStreamWriter& w,
				const QStringList& authors,
				const QStringList& genres,
				const QString& name,
				const ICoreProxy_ptr& proxy)
		{
			w.setAutoFormatting (true);
			w.setAutoFormattingIndent (2);
			w.writeStartDocument ();
			w.writeStartElement ("FictionBook");
			w.writeDefaultNamespace ("http://www.gribuser.ru/xml/fictionbook/2.0");
			w.writeNamespace ("http://www.w3.org/1999/xlink", "l");

			w.writeStartElement ("description");
				w.writeStartElement ("title-info");
					for (const auto& genre : genres)
						w.writeTextElement ("genre", genre);
					for (const auto& author : authors)
					{
						w.writeStartElement ("author");
							w.writeTextElement ("nickname", author);
						w.writeEndElement ();
					}
					w.writeTextElement ("book-title", name);
					w.writeTextElement ("lang", "en");
				w.writeEndElement ();

				w.writeStartElement ("document-info");
					w.writeStartElement ("author");
						w.writeTextElement ("nickname", "LeechCraft");
					w.writeEndElement ();
					w.writeTextElement ("program-used",
							QString ("LeechCraft Aggregator %1")
								.arg (proxy->GetVersion ()));
					w.writeTextElement ("id",
							QUuid::createUuid ().toString ());
					w.writeTextElement ("version", "1.0");
					w.writeStartElement ("date");
						const QDate& date = QDate::currentDate ();
						w.writeAttribute ("date", date.toString (Qt::ISODate));
						w.writeCharacters (date.toString (Qt::TextDate));
					w.writeEndElement ();
				w.writeEndElement ();
			w.writeEndElement ();

			w.writeStartElement ("body");
		}
	}

	void Export2FB2Dialog::WriteFB2 (const WriteInfo& info)
	{
		QFile file (info.File_);
		if (!file.open (QIODevice::WriteOnly))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not open file %1 for write:<br />%2.")
						.arg (Ui_.File_->text ())
						.arg (file.errorString ()));
			return;
		}

		const auto& channels = info.Items_.keys ();
		auto authors = Util::Map (channels, &ChannelShort::Author_);
		if (!authors.size ())
			authors << "LeechCraft";
		else
			authors.removeDuplicates ();

		QStringList genres;
		for (int i = 0; i < Ui_.FB2Genres_->topLevelItemCount (); ++i)
		{
			auto item = Ui_.FB2Genres_->topLevelItem (i);
			for (int j = 0; j < item->childCount (); ++j)
			{
				auto subItem = item->child (j);
				if (subItem->checkState (0) == Qt::Checked)
					genres << subItem->text (1);
			}
		}

		QXmlStreamWriter w (&file);
		WriteBeginning (w, authors, genres, Ui_.Name_->text (), Proxy_);

		for (const auto& cs : channels)
			WriteChannel (w, cs, info.Items_ [cs]);
		w.writeEndElement ();
		w.writeEndDocument ();

		const auto iem = Proxy_->GetEntityManager ();
		iem->HandleEntity (Util::MakeNotification ("Aggregator",
					tr ("Export complete."),
					Priority::Info));
	}

	namespace
	{
		void WritePDFChannel (QTextCursor& cursor,
				const ChannelShort& cs, const QList<Item>& items,
				const QTextFrame *topFrame, int baseFontSize, const QFont& font)
		{
			auto origCharFmt = cursor.charFormat ();
			origCharFmt.setFontPointSize (baseFontSize);

			QTextFrameFormat frameFmt;
			frameFmt.setBorder (1);
			frameFmt.setPadding (5);
			frameFmt.setBackground (QColor ("#A4C0E4"));
			cursor.insertFrame (frameFmt);

			auto titleFmt = origCharFmt;
			titleFmt.setFontWeight (QFont::Bold);
			titleFmt.setFontPointSize (baseFontSize * 1.35);
			cursor.setCharFormat (titleFmt);
			cursor.insertText (cs.Title_);

			cursor.setPosition (topFrame->lastPosition ());

			titleFmt.setFontWeight (QFont::DemiBold);
			titleFmt.setFontPointSize (baseFontSize * 1.20);
			auto dateFmt = origCharFmt;
			dateFmt.setFontItalic (true);
			for (const auto& item : items)
			{
				cursor.setCharFormat (titleFmt);
				cursor.insertText (item.Title_ + "\n");

				cursor.setCharFormat (dateFmt);
				cursor.insertText (item.PubDate_.toString ());

				cursor.setCharFormat (origCharFmt);

				auto descr = item.Description_;
				QRegExp imgRx ("<img *>", Qt::CaseSensitive, QRegExp::Wildcard);
				imgRx.setMinimal (true);
				descr.remove (imgRx);
				descr.remove ("</img>");

				QRegExp linkRx ("<a.*></a>");
				linkRx.setMinimal (true);
				descr.remove (linkRx);
				descr += "<br/><br/>";

				descr.prepend (QString ("<div style='font-size: %1pt; font-family: %2;'>")
							.arg (baseFontSize)
							.arg (font.family ()));
				descr.append ("</div>");

				cursor.insertHtml (descr);
			}
		}
	}

	void Export2FB2Dialog::WritePDF (const WriteInfo& info)
	{
		QTextDocument doc;
		QTextCursor cursor (&doc);

		const auto& font = Ui_.PDFFont_->currentFont ();
		const int baseFontSize = Ui_.PDFFontSize_->value ();

		auto topFrame = cursor.currentFrame ();
		QTextFrameFormat frameFmt;
		frameFmt.setBorder (1);
		frameFmt.setPadding (10);
		frameFmt.setBackground (QColor ("#A4C0E4"));
		cursor.insertFrame(frameFmt);

		auto origFmt = cursor.charFormat ();
		origFmt.setFont (font);
		origFmt.setFontPointSize (baseFontSize);

		auto titleFmt = origFmt;
		titleFmt.setFontWeight (QFont::Bold);
		titleFmt.setFontPointSize (baseFontSize * 1.5);
		cursor.setCharFormat (titleFmt);
		cursor.insertText (Ui_.Name_->text ());

		cursor.setPosition (topFrame->lastPosition ());

		cursor.setCharFormat (origFmt);
		for (const auto& pair : Util::Stlize (info.Items_))
			WritePDFChannel (cursor, pair.first, pair.second, topFrame, baseFontSize, font);

		QPrinter printer;
		printer.setOutputFileName (info.File_);
		printer.setOutputFormat (QPrinter::PdfFormat);
		printer.setFontEmbeddingEnabled (true);
		printer.setPageMargins (Ui_.LeftMargin_->value (), Ui_.TopMargin_->value (),
				Ui_.RightMargin_->value (), Ui_.BottomMargin_->value (),
				QPrinter::Millimeter);

		switch (Ui_.PageSizeBox_->currentIndex ())
		{
		case 0:
			printer.setPageSize (QPrinter::A4);
			break;
		case 1:
			printer.setPageSize (QPrinter::A5);
			break;
		case 2:
			printer.setPageSize (QPrinter::Letter);
			break;
		}

		doc.print (&printer);

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Aggregator",
					tr ("Export complete."),
					Priority::Info));
	}

	void Export2FB2Dialog::on_Browse__released ()
	{
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Select save file"),
				QDir::homePath () + "/export.fb2",
				tr ("fb2 files (*.fb2);;XML files (*.xml);;PDF files (*.pdf);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		Ui_.File_->setText (filename);

		if (filename.endsWith (".pdf"))
			Ui_.ExportFormat_->setCurrentIndex (1);
		else
			Ui_.ExportFormat_->setCurrentIndex (0);
	}

	void Export2FB2Dialog::on_File__textChanged (const QString& name)
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (!name.isEmpty ());
	}

	void Export2FB2Dialog::on_Name__textEdited ()
	{
		HasBeenTextModified_ = true;
	}

	namespace
	{
		QStringList CatsFromIndexes (const QList<QModelIndex>& indices)
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

			return Util::ConcatMap (indices,
					[&sb] (const QModelIndex& index)
					{
						const auto channelId = index.data (ChannelRoles::ChannelID).value<IDType_t> ();
						return ItemUtils::GetCategories (sb->GetItems (channelId));
					}).values ();
		}
	}

	void Export2FB2Dialog::handleChannelsSelectionChanged ()
	{
		Selector_->SetPossibleSelections (CatsFromIndexes (Ui_.ChannelsTree_->selectionModel ()->selectedRows ()));
		Selector_->selectAll ();

		if (!HasBeenTextModified_ &&
				Ui_.ChannelsTree_->selectionModel ()->selectedRows ().size () <= 1)
		{
			const QModelIndex& index = Ui_.ChannelsTree_->currentIndex ();
			if (index.isValid ())
				Ui_.Name_->setText (index.sibling (index.row (), 0).data ().toString ());
		}
	}

	void Export2FB2Dialog::handleAccepted ()
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		bool unreadOnly = Ui_.UnreadOnly_->checkState () == Qt::Checked;
		const auto& categories = Selector_->GetSelections ();

		QMap<ChannelShort, QList<Item>> items2write;

		for (const auto& row : Ui_.ChannelsTree_->selectionModel ()->selectedRows ())
		{
			const auto& cs = ChannelsModel_->GetChannelForIndex (row);

			const auto& items = sb->GetItems (cs.ChannelID_);

			for (const auto& item : items)
			{
				if (unreadOnly &&
						!item.Unread_)
					continue;

				if (!item.Categories_.isEmpty ())
				{
					const auto suitable = std::any_of (categories.begin (), categories.end (),
							[&item] (const QString& cat) { return item.Categories_.contains (cat); });
					if (!suitable)
						continue;
				}

				if (const auto& fullItem = sb->GetItem (item.ItemID_))
					items2write [cs].prepend (*fullItem);
			}
		}

		const WriteInfo info = { Ui_.File_->text (), items2write };

		switch (Ui_.ExportFormat_->currentIndex ())
		{
		case 0:
			WriteFB2 (info);
			break;
		case 1:
			WritePDF (info);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown format ID"
					<< Ui_.ExportFormat_->currentIndex ();
			return;
		}
	}
}
}
