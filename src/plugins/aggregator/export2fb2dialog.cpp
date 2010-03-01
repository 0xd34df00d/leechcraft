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

#include "export2fb2dialog.h"
#include <QXmlStreamWriter>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>
#include <QDate>
#include <plugininterface/categoryselector.h>
#include "core.h"
#include "channelsmodel.h"
#include "config.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Export2FB2Dialog::Export2FB2Dialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				Ui_.ChannelsTree_->setModel (Core::Instance ().GetRawChannelsModel ());

				Selector_ = new Util::CategorySelector (this);
				Selector_->setWindowFlags (Qt::Widget);
				Selector_->SetPossibleSelections (QStringList ());
				Ui_.VLayout_->addWidget (Selector_);

				connect (Ui_.ChannelsTree_->selectionModel (),
						SIGNAL (selectionChanged (const QItemSelection&,
								const QItemSelection&)),
						this,
						SLOT (handleChannelsSelectionChanged (const QItemSelection&,
								const QItemSelection&)));

				connect (this,
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));

				on_File__textChanged (QString ());
			}

			void Export2FB2Dialog::on_Browse__released ()
			{
				QString filename = QFileDialog::getSaveFileName (this,
						tr ("Select save file"),
						QDir::homePath () + "/export.fb2",
						tr ("fb2 files (*.fb2);;XML files (*.xml);;All files (*.*)"));
				if (filename.isEmpty ())
					return;

				Ui_.File_->setText (filename);
			}

			void Export2FB2Dialog::on_File__textChanged (const QString& name)
			{
				Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (name.size ());
			}

			void Export2FB2Dialog::handleChannelsSelectionChanged (const QItemSelection& selected,
					const QItemSelection& deselected)
			{
				QStringList removedCategories;
				Q_FOREACH (QModelIndex index, deselected.indexes ())
					removedCategories << Core::Instance ().GetCategories (index);
				removedCategories.removeDuplicates ();
				Q_FOREACH (QString removed, removedCategories)
					CurrentCategories_.removeAll (removed);

				QStringList addedCategories;
				Q_FOREACH (QModelIndex index, selected.indexes ())
					addedCategories << Core::Instance ().GetCategories (index);
				CurrentCategories_ << addedCategories;
				
				CurrentCategories_.removeDuplicates ();

				Selector_->SetPossibleSelections (CurrentCategories_);
				Selector_->selectAll ();
			}

			void WriteChannel (QXmlStreamWriter& w,
					const ChannelShort& cs, const QList<Item_ptr>& items)
			{
				w.writeStartElement ("section");
					w.writeAttribute ("id", cs.ParentURL_ + cs.Title_);
					w.writeStartElement ("title");
						w.writeTextElement ("p", cs.Title_);
					w.writeEndElement ();
					w.writeTextElement ("annotation",
							Export2FB2Dialog::tr ("%1 unread items")
								.arg (cs.Unread_));
					Q_FOREACH (Item_ptr item, items)
					{
						w.writeStartElement ("title");
							w.writeTextElement ("p", item->Title_);
						w.writeEndElement ();

						bool hasDate = item->PubDate_.isValid ();
						bool hasAuthor = item->Author_.size ();
						if (hasDate || hasAuthor)
						{
							w.writeStartElement ("epigraph");
								if (hasDate)
									w.writeTextElement ("p",
											Export2FB2Dialog::tr ("Published on %1")
												.arg (item->PubDate_.toString ()));
								if (hasAuthor)
									w.writeTextElement ("p",
											Export2FB2Dialog::tr ("By %1")
												.arg (item->Author_));
							w.writeEndElement ();
							w.writeEmptyElement ("empty-line");
						}

						QString descr = item->Description_;
						descr.remove ("<p>");
						descr.remove ("</p>");
						descr.remove (QRegExp ("<img *>",
									Qt::CaseSensitive, QRegExp::Wildcard));
						descr.remove (QRegExp ("<a *>",
									Qt::CaseSensitive, QRegExp::Wildcard));
						w.writeTextElement ("p", descr);
						w.writeEmptyElement ("empty-line");
					}
				w.writeEndElement ();
			}

			void WriteBeginning (QXmlStreamWriter& w,
					const QStringList& authors)
			{
				w.setAutoFormatting (true);
				w.setAutoFormattingIndent (2);
				w.writeStartDocument ();
				w.writeStartElement ("FictionBook");
				w.writeDefaultNamespace ("http://www.gribuser.ru/xml/fictionbook/2.0");
				w.writeNamespace ("http://www.w3.org/1999/xlink", "l");

				w.writeStartElement ("description");
					w.writeStartElement ("title-info");
						w.writeTextElement ("genre", "comp_www");
						w.writeTextElement ("genre", "computers");
						Q_FOREACH (QString author, authors)
						{
							w.writeStartElement ("author");
								w.writeTextElement ("nickname", author);
							w.writeEndElement ();
						}
						w.writeTextElement ("book-title", "Exported Feeds");
						w.writeTextElement ("lang", "en");
					w.writeEndElement ();

					w.writeStartElement ("document-info");
						w.writeStartElement ("author");
							w.writeTextElement ("nickname", "LeechCraft");
						w.writeEndElement ();
						w.writeTextElement ("program-used",
								QString ("LeechCraft Aggregator %1")
									.arg (LEECHCRAFT_VERSION));
						w.writeTextElement ("id",
								QUuid::createUuid ().toString ());
						w.writeTextElement ("version", "1.0");
						w.writeTextElement ("date",
								QDate::currentDate ().toString (Qt::ISODate));
					w.writeEndElement ();
				w.writeEndElement ();

				w.writeStartElement ("body");
			}

			void Export2FB2Dialog::handleAccepted ()
			{
				QFile file (Ui_.File_->text ());
				if (!file.open (QIODevice::WriteOnly))
				{
					QMessageBox::critical (this,
							tr ("LeechCraft"),
							tr ("Could not open file %1 for write:<br />%2.")
								.arg (Ui_.File_->text ())
								.arg (file.errorString ()));
					return;
				}

				StorageBackend *sb = Core::Instance ().GetStorageBackend ();

				QModelIndexList rows = Ui_.ChannelsTree_->selectionModel ()->selectedRows ();
				bool unreadOnly = Ui_.UnreadOnly_->checkState () == Qt::Checked;
				QStringList categories = Selector_->GetSelections ();

				QMap<ChannelShort, QList<Item_ptr> > items2write;
				QStringList authors;

				Q_FOREACH (QModelIndex row, rows)
				{
					ChannelShort cs = Core::Instance ()
						.GetRawChannelsModel ()->GetChannelForIndex (row);

					Channel_ptr channel = sb->GetChannel (cs.Title_,
							cs.ParentURL_);
					authors << channel->Author_;

					items_shorts_t items;
					QString hash = cs.ParentURL_ + cs.Title_;
					sb->GetItems (items, hash);
			
					for (items_shorts_t::const_iterator i = items.begin (),
							end = items.end (); i != end; ++i)
					{
						if (unreadOnly &&
								!i->Unread_)
							continue;

						if (!i->Categories_.isEmpty ())
						{
							bool suitable = false;
							Q_FOREACH (QString cat, categories)
								if (i->Categories_.contains (cat))
								{
									suitable = true;
									break;
								}

							if (!suitable)
								continue;
						}

						Item_ptr item = sb->GetItem (i->Title_,
									i->URL_, hash);

						items2write [cs].prepend (item);
					}
				}

				if (!authors.size ())
					authors << "LeechCraft";

				QXmlStreamWriter w (&file);
				WriteBeginning (w, authors);

				QList<ChannelShort> shorts = items2write.keys ();
				Q_FOREACH (ChannelShort cs, shorts)
					WriteChannel (w, cs, items2write [cs]);
				w.writeEndElement ();
				w.writeEndDocument ();
			}
		};
	};
};

