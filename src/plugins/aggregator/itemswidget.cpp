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

#include "itemswidget.h"
#include <memory>
#include <QFileInfo>
#include <QHeaderView>
#include <QtDebug>
#include <QUrl>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/categoryselector.h>
#include <plugininterface/util.h>
#include "core.h"
#include "itemsfiltermodel.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			using LeechCraft::Util::CategorySelector;
			
			struct ItemsWidget_Impl
			{
				Ui::ItemsWidget Ui_;

				QAction *ActionMarkItemAsUnread_;
				QAction *ActionAddToItemBucket_;
				QAction *ActionItemCommentsSubscribe_;

				bool TapeMode_;

				std::auto_ptr<ItemsFilterModel> ItemsFilterModel_;
				std::auto_ptr<CategorySelector> ItemCategorySelector_;
			};
			
			ItemsWidget::ItemsWidget (QWidget *parent)
			: QWidget (parent)
			{
				Impl_ = new ItemsWidget_Impl;
				Impl_->TapeMode_ = false;
				Impl_->ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"),
						this);
				Impl_->ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");
			
				Impl_->ActionAddToItemBucket_ = new QAction (tr ("Add to item bucket"),
						this);
				Impl_->ActionAddToItemBucket_->setObjectName ("ActionAddToItemBucket_");

				Impl_->ActionItemCommentsSubscribe_ = new QAction (tr ("Subscribe to comments"),
						this);
				Impl_->ActionItemCommentsSubscribe_->setObjectName ("ActionItemCommentsSubscribe_");
			
				Impl_->Ui_.setupUi (this);
				Impl_->Ui_.ItemView_->Construct (Core::Instance ().GetWebBrowser ());
			
				Impl_->Ui_.Items_->setAcceptDrops (false);
			
				Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);
				Impl_->ItemsFilterModel_.reset (new ItemsFilterModel (this));
				Impl_->ItemsFilterModel_->setSourceModel (Core::Instance ().GetItemsModel ());
				connect (Core::Instance ().GetItemsModel (),
						SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
						Impl_->ItemsFilterModel_.get (),
						SLOT (invalidate ()));
				Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
				Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
				Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());
			
				Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsUnread_);
				Impl_->Ui_.Items_->addAction (Impl_->ActionAddToItemBucket_);
				Impl_->Ui_.Items_->addAction (Impl_->ActionItemCommentsSubscribe_);
				Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);
				connect (Impl_->Ui_.SearchLine_,
						SIGNAL (textChanged (const QString&)),
						this,
						SLOT (updateItemsFilter ()));
				connect (Impl_->Ui_.SearchType_,
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (updateItemsFilter ()));
			
				connect (&Core::Instance (),
						SIGNAL (currentChannelChanged (const QModelIndex&)),
						this,
						SLOT (channelChanged (const QModelIndex&)));
			
				QHeaderView *itemsHeader = Impl_->Ui_.Items_->header ();
				QFontMetrics fm = fontMetrics ();
				int dateTimeSize = fm.width (QDateTime::currentDateTime ()
						.toString (Qt::SystemLocaleShortDate) + "__");
				itemsHeader->resizeSection (0,
						fm.width ("Average news article size is about this width or "
							"maybe bigger, because they are bigger"));
				itemsHeader->resizeSection (1,
						dateTimeSize);
				connect (Impl_->Ui_.Items_->header (),
						SIGNAL (sectionClicked (int)),
						this,
						SLOT (makeCurrentItemVisible ()));
			
				Impl_->ItemCategorySelector_.reset (new CategorySelector ());
				Impl_->ItemCategorySelector_->setWindowFlags (Qt::Widget);
				Impl_->Ui_.CategoriesSplitter_->addWidget (Impl_->ItemCategorySelector_.get ());
				Impl_->ItemCategorySelector_->hide ();
				Impl_->ItemCategorySelector_->setMinimumHeight (0);
				connect (Impl_->ItemCategorySelector_.get (),
						SIGNAL (selectionChanged (const QStringList&)),
						Impl_->ItemsFilterModel_.get (),
						SLOT (categorySelectionChanged (const QStringList&)));
			
				connect (Impl_->Ui_.Items_->selectionModel (),
						SIGNAL (selectionChanged (const QItemSelection&,
								const QItemSelection&)),
						this,
						SLOT (currentItemChanged (const QItemSelection&)),
						Qt::QueuedConnection);
			
				currentItemChanged (QItemSelection ());

				XmlSettingsManager::Instance ()->RegisterObject ("ShowCategorySelector",
						this, "selectorVisiblityChanged");
			}
			
			ItemsWidget::~ItemsWidget ()
			{
				on_CategoriesSplitter__splitterMoved ();

				disconnect (Impl_->ItemsFilterModel_.get (),
						0,
						this,
						0);
				disconnect (Impl_->ItemCategorySelector_.get (),
						0,
						this,
						0);
				delete Impl_;
			}
			
			void ItemsWidget::SetTapeMode (bool tape)
			{
				if (!isVisible ())
					return;

				Impl_->TapeMode_ = tape;
				if (tape)
					disconnect (Impl_->Ui_.Items_->selectionModel (),
							SIGNAL (selectionChanged (const QItemSelection&,
									const QItemSelection&)),
							this,
							SLOT (currentItemChanged (const QItemSelection&)));
				else
					connect (Impl_->Ui_.Items_->selectionModel (),
							SIGNAL (selectionChanged (const QItemSelection&,
									const QItemSelection&)),
							this,
							SLOT (currentItemChanged (const QItemSelection&)),
							Qt::QueuedConnection);
				currentItemChanged (QItemSelection ());
			}
			
			void ItemsWidget::SetHideRead (bool hide)
			{
				Impl_->ItemsFilterModel_->SetHideRead (hide);
			}
			
			QString ItemsWidget::GetHex (QPalette::ColorRole role, QPalette::ColorGroup group)
			{
				int r, g, b;
				QApplication::palette ().color (group, role).getRgb (&r, &g, &b);
				long color = b + (g << 8) + (b << 16);
				QString result ("#%1");
				// Fill spare space with zeros.
				return result.arg (color, 6, 16, QChar ('0'));
			}
			
			QString ItemsWidget::ToHtml (const Item_ptr& item)
			{
				QPalette palette = QApplication::palette ();
			
				QString headerBg = GetHex (QPalette::Window);
				QString headerText = GetHex (QPalette::WindowText);
				QString alternateBg = GetHex (QPalette::AlternateBase);
			
				QString startBox = "<div style='background: %1; "
					"color: COLOR; "
					"margin-left: -1em;"
					"margin-right: -1em; "
					"padding-left: 2em; "
					"padding-right: 2em;'>";
				startBox.replace ("COLOR", headerText);
			
				bool linw = XmlSettingsManager::Instance ()->
						property ("AlwaysUseExternalBrowser").toBool ();
			
				QString result = QString ("<div style='background: %1; "
						"margin-left: 0px; "
						"margin-right: 0px; "
						"padding-left: 1em; "
						"padding-right: 1em'>")
					.arg (GetHex (QPalette::Base));

				QString inpad = QString ("<div style='background: %1; "
						"color: %2; "
						"border: 1px solid #333333; "
						"padding-top: 1em; "
						"padding-bottom: 1em; "
						"padding-left: 2em; "
						"padding-right: 2em;'>");
			
				// Title
				result += startBox.arg (headerBg);
				result += (QString ("<strong>") +
						item->Title_ +
						"</strong></div>");
			
				// Link
				result += (startBox.arg (headerBg) +
						"<a href='" +
						item->Link_ +
						"'");
				if (linw)
					result += " target='_blank'";
				result += (QString (">") +
						item->Link_ +
						"</a></div>");
			
				// Publication date and author
				if (item->PubDate_.isValid () && !item->Author_.isEmpty ())
					result += (startBox.arg (headerBg) +
							tr ("Published on %1 by %2")
						   		.arg (item->PubDate_.toString ())
								.arg (item->Author_) +
							"</div>");
				else if (item->PubDate_.isValid ())
					result += (startBox.arg (headerBg) +
							tr ("Published on %1")
						   		.arg (item->PubDate_.toString ()) +
							"</div>");
				else if (!item->Author_.isEmpty ())
					result += (startBox.arg (headerBg) +
							tr ("Published by %1")
								.arg (item->Author_) +
							"</div>");
			
				// Categories
				if (item->Categories_.size ())
					result += (startBox.arg (headerBg) +
							item->Categories_.join ("; ") +
							"</div>");
			
				// Comments stuff
				if (item->NumComments_ >= 0 && !item->CommentsPageLink_.isEmpty ())
					result += (startBox.arg (headerBg) + 
							tr ("%1 comments, <a href='%2'%3>view them</a></div>")
								.arg (item->NumComments_)
								.arg (item->CommentsPageLink_)
								.arg (linw ? " target='_blank'" : ""));
				else if (item->NumComments_ >= 0)
					result += (startBox.arg (headerBg) + 
							tr ("%1 comments</div>")
								.arg (item->NumComments_));
				else if (!item->CommentsPageLink_.isEmpty ())
					result += (startBox.arg (headerBg) + 
							tr ("<a href='%1'%2>View comments</a></div>")
								.arg (item->CommentsPageLink_)
								.arg (linw ? " target='_blank'" : ""));

				if (item->Latitude_ ||
						item->Longitude_)
				{
					QString link = QString ("http://maps.google.com/maps"
							"?f=q&source=s_q&hl=en&geocode=&q=%1+%2")
						.arg (item->Latitude_)
						.arg (item->Longitude_);
					result += (startBox.arg (headerBg) +
							tr ("Geoposition: <a href='%3'%4 title='Google Maps'>%1 %2</a></div>")
								.arg (item->Latitude_)
								.arg (item->Longitude_)
								.arg (link)
								.arg (linw ? " target='_blank'" : ""));
				}
			
				result += "<br />";
				result += QString ("<div style='color: %2'>")
					.arg (GetHex (QPalette::Text));
			
				// Description
				result += item->Description_;

				for (QList<Enclosure>::const_iterator i = item->Enclosures_.begin (),
						end = item->Enclosures_.end (); i != end; ++i)
				{
					result += inpad.arg (headerBg)
						.arg (headerText);
					if (i->Length_ > 0)
						result += tr ("File of type %1, size %2:<br />")
							.arg (i->Type_)
							.arg (LeechCraft::Util::MakePrettySize (i->Length_));
					else
						result += tr ("File of type %1 and unknown length:<br />")
							.arg (i->Type_);
					result += QString ("<a href='%1'>%2</a>")
						.arg (i->URL_)
						.arg (QFileInfo (QUrl (i->URL_).path ()).fileName ());
					if (!i->Lang_.isEmpty ())
						result += tr ("<br />Specified language: %1")
							.arg (i->Lang_);
					result += "</div>";
				}

				for (QList<MRSSEntry>::const_iterator entry = item->MRSSEntries_.begin (),
						endEntry = item->MRSSEntries_.end (); entry != endEntry; ++entry)
				{
					result += inpad.arg (headerBg)
						.arg (headerText);

					QString url = entry->URL_;

					if (entry->Medium_ == "image")
						result += tr ("Image ");
					else if (entry->Medium_ == "audio")
						result += tr ("Audio ");
					else if (entry->Medium_ == "video")
						result += tr ("Video ");
					else if (entry->Medium_ == "document")
						result += tr ("Document ");
					else if (entry->Medium_ == "executable")
						result += tr ("Executable ");

					if (entry->Title_.isEmpty ())
						result += tr ("<a href='%1' target='_blank'>%1</a><hr />")
							.arg (url);
					else
						result += tr ("<a href='%1' target='_blank'>%2</a><hr />")
							.arg (url)
							.arg (entry->Title_);

					if (entry->Size_ > 0)
					{
						result += Util::MakePrettySize (entry->Size_);
						result += "<br />";
					}

					QString peers;
					Q_FOREACH (MRSSPeerLink pl, entry->PeerLinks_)
						peers += QString ("<li>Also available in <a href='%1'>P2P (%2)</a></li>")
							.arg (pl.Link_)
							.arg (pl.Type_);
					if (peers.size ())
					{
						result += inpad.arg (alternateBg)
							.arg (headerText);
						result += QString ("<ul>%1</ul>")
							.arg (peers);
						result += "</div>";
					}

					if (!entry->Description_.isEmpty ())
						result += tr ("%1<br />")
							.arg (entry->Description_);

					QList<int> sizes;
					int num = 0;
					for (int i = 0; i < entry->Thumbnails_.size (); ++i)
					{
						int width = entry->Thumbnails_.at (i).Width_;
						if (!width)
							break;

						if (!sizes.contains (width))
							sizes << width;
						else
						{
							bool broke = false;;
							for (int j = i + 1; j < entry->Thumbnails_.size (); ++j)
								if (entry->Thumbnails_.at (j).Width_ == sizes.at (j % sizes.size ()))
								{
									broke = true;
									break;
								}

							if (broke)
								continue;
							num = sizes.size ();
							break;
						}
					}

					if (!num || num == entry->Thumbnails_.size ())
						num = 3;

					int cur = 1;
					Q_FOREACH (MRSSThumbnail thumb, entry->Thumbnails_)
					{
						if (!thumb.Time_.isEmpty ())
							result += tr ("<hr />Thumbnail at %1:<br />")
								.arg (thumb.Time_);
						result += QString ("<img src='%1' ")
							.arg (thumb.URL_);
						if (thumb.Width_)
							result += QString ("width='%1' ")
								.arg (thumb.Width_);
						if (thumb.Height_)
							result += QString ("height='%1' ")
								.arg (thumb.Height_);
						result += "/>";

						if (num && cur < num)
							++cur;
						else
						{
							result += "<br />";
							cur = 1;
						}
					}

					result += "<hr />";

					if (!entry->Keywords_.isEmpty ())
						result += tr ("<strong>Keywords:</strong> <em>%1</em><br />")
							.arg (entry->Keywords_);

					if (!entry->Lang_.isEmpty ())
						result += tr ("<strong>Language:</strong> %1<br />")
							.arg (entry->Lang_);

					if (entry->Expression_ == "sample")
						result += tr ("Sample");
					else if (entry->Expression_ == "nonstop")
						result += tr ("Continuous stream");
					else
						result += tr ("Full version");
					result += "<br />";

					QString scenes;
					Q_FOREACH (MRSSScene sc, entry->Scenes_)
					{
						QString current;
						if (!sc.Title_.isEmpty ())
							current += tr ("Title: %1<br />")
								.arg (sc.Title_);
						if (!sc.StartTime_.isEmpty ())
							current += tr ("Start time: %1<br />")
								.arg (sc.StartTime_);
						if (!sc.EndTime_.isEmpty ())
							current += tr ("End time: %1<br />")
								.arg (sc.EndTime_);
						if (!sc.Description_.isEmpty ())
							current += tr ("%1<br />")
								.arg (sc.Description_);

						if (!current.isEmpty ())
							scenes += QString ("<li>%1</li>")
								.arg (current);
					}

					if (scenes.size ())
					{
						result += tr ("<strong>Scenes:</strong>");
						result += inpad.arg (alternateBg)
							.arg (headerText);
						result += QString ("<ul>%1</ul>")
							.arg (scenes);
						result += "</div>";
					}

					if (entry->Views_)
						result += tr ("<strong>Views:</strong> %1")
							.arg (entry->Views_);
					if (entry->Favs_)
						result += tr ("<strong>Added to favorites:</strong> %1 times")
							.arg (entry->Favs_);
					if (entry->RatingAverage_)
						result += tr ("<strong>Average rating:</strong> %1")
							.arg (entry->RatingAverage_);
					if (entry->RatingCount_)
						result += tr ("<strong>Number of marks:</strong> %1")
							.arg (entry->RatingCount_);
					if (entry->RatingMin_)
						result += tr ("<strong>Minimal rating:</strong> %1")
							.arg (entry->RatingMin_);
					if (entry->RatingMax_)
						result += tr ("<strong>Maximal rating:</strong> %1")
							.arg (entry->RatingMax_);

					if (!entry->Tags_.isEmpty ())
						result += tr ("<strong>User tags:</strong> %1")
							.arg (entry->Tags_);

					QString tech;
					if (entry->Duration_)
						tech += tr ("<li><strong>Duration:</strong> %1</li>")
							.arg (entry->Channels_);
					if (entry->Channels_)
						tech += tr ("<li><strong>Channels:</strong> %1</li>")
							.arg (entry->Channels_);
					if (entry->Width_ &&
							entry->Height_)
						tech += tr ("<li><strong>Size:</strong> %1x%2</li>")
							.arg (entry->Width_)
							.arg (entry->Height_);
					if (entry->Bitrate_)
						tech += tr ("<li><strong>Bitrate:</strong> %1 kbps</li>")
							.arg (entry->Bitrate_);
					if (entry->Framerate_)
						tech += tr ("<li><strong>Framerate:</strong> %1</li>")
							.arg (entry->Framerate_);
					if (entry->SamplingRate_)
						tech += tr ("<li><strong>Sampling rate:</strong> %1</li>")
							.arg (entry->SamplingRate_);
					if (!entry->Type_.isEmpty ())
						tech += tr ("<li><strong>MIME type:</strong> %1</li>")
							.arg (entry->Type_);

					if (!tech.isEmpty ())
					{
						result += tr ("<strong>Technical information:</strong>");
						result += inpad.arg (alternateBg)
							.arg (headerText);
						result += QString ("<ul>%1</ul>")
							.arg (tech);
						result += "</div>";
					}

					if (!entry->Rating_.isEmpty () &&
							!entry->RatingScheme_.isEmpty ())
						result += tr ("<strong>Rating:</strong> %1 (according to %2 scheme)<br />")
							.arg (entry->Rating_)
							.arg (entry->RatingScheme_.mid (4));

					QMap<QString, QString> comments;
					Q_FOREACH (MRSSComment cm, entry->Comments_)
						comments [cm.Type_] += QString ("<li>%1</li>")
							.arg (cm.Comment_);

					QStringList cmTypes = comments.keys ();
					Q_FOREACH (QString type, cmTypes)
					{
						result += tr ("<strong>%1:</strong>")
							.arg (type);
						result += inpad.arg (alternateBg)
							.arg (headerText);
						result += QString ("<ul>%1</ul>")
							.arg (comments [type]);
						result += "</div>";
					}

					if (!entry->CopyrightURL_.isEmpty ())
					{
						if (!entry->CopyrightText_.isEmpty ())
							result += tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%2</a><br />")
								.arg (entry->CopyrightURL_)
								.arg (entry->CopyrightText_);
						else
							result += tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%1</a><br />")
								.arg (entry->CopyrightURL_);
					}
					else if (!entry->CopyrightText_.isEmpty ())
						result += tr ("<strong>Copyright:</strong> %1<br />")
							.arg (entry->CopyrightText_);

					QString credits;
					Q_FOREACH (MRSSCredit cr, entry->Credits_)
					{
						if (cr.Role_.isEmpty ())
							continue;
						credits += tr ("<li>%1: %2</li>")
							.arg (cr.Role_)
							.arg (cr.Who_);
					}

					if (!credits.isEmpty ())
					{
						result += tr ("<strong>Credits:</strong>");
						result += inpad.arg (alternateBg)
							.arg (headerText);
						result += QString ("<ul>%1</ul>")
							.arg (credits);
						result += "</div>";
					}

					result += "</div>";
				}
			
				result += "</div>";
				result += "</div>";
			
				return result;
			}

			void ItemsWidget::RestoreSplitter ()
			{
				QList<int> sizes;
				sizes << XmlSettingsManager::Instance ()->
					Property ("CategoriesSplitter1", 0).toInt ();
				sizes << XmlSettingsManager::Instance ()->
					Property ("CategoriesSplitter2", 0).toInt ();
				if (!sizes.at (0) &&
						!sizes.at (1))
				{
					Impl_->Ui_.CategoriesSplitter_->setStretchFactor (0, 8);
					Impl_->Ui_.CategoriesSplitter_->setStretchFactor (1, 1);
				}
				else
					Impl_->Ui_.CategoriesSplitter_->setSizes (sizes);
			}
			
			void ItemsWidget::channelChanged (const QModelIndex& mapped)
			{
				Impl_->Ui_.Items_->scrollToTop ();
				currentItemChanged (QItemSelection ());

				if (!isVisible ())
					return;
			
				QStringList allCategories = Core::Instance ().GetCategories (mapped);
				Impl_->ItemsFilterModel_->categorySelectionChanged (allCategories);

				if (allCategories.size ())
				{
					Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
					Impl_->ItemCategorySelector_->selectAll ();
					if (XmlSettingsManager::Instance ()->
							property ("ShowCategorySelector").toBool ())
					Impl_->ItemCategorySelector_->show ();
					RestoreSplitter ();
				}
				else
				{
					Impl_->ItemCategorySelector_->SetPossibleSelections (QStringList ());
					Impl_->ItemCategorySelector_->hide ();
				}
			}
			
			void ItemsWidget::on_ActionMarkItemAsUnread__triggered ()
			{
				QModelIndexList indexes = Impl_->Ui_.Items_->
					selectionModel ()->selectedRows ();
				for (int i = 0; i < indexes.size (); ++i)
					Core::Instance ().MarkItemAsUnread (Impl_->
							ItemsFilterModel_->mapToSource (indexes.at (i)));
			}
			
			void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
			{
				Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
						Qt::CaseSensitive : Qt::CaseInsensitive);
			}
			
			void ItemsWidget::on_ActionAddToItemBucket__triggered ()
			{
				Core::Instance ().AddToItemBucket (Impl_->ItemsFilterModel_->
						mapToSource (Impl_->Ui_.Items_->selectionModel ()->
							currentIndex ()));
			}
			
			void ItemsWidget::on_ActionItemCommentsSubscribe__triggered ()
			{
				QModelIndex selected = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
				Core::Instance ().SubscribeToComments (Impl_->ItemsFilterModel_->
						mapToSource (selected));
			}

			void ItemsWidget::on_CategoriesSplitter__splitterMoved ()
			{
				QList<int> sizes = Impl_->Ui_.CategoriesSplitter_->sizes ();
				XmlSettingsManager::Instance ()->
					setProperty ("CategoriesSplitter1", sizes.at (0));
				XmlSettingsManager::Instance ()->
					setProperty ("CategoriesSplitter2", sizes.at (1));
			}
			
			void ItemsWidget::currentItemChanged (const QItemSelection& selection)
			{
				if (Impl_->TapeMode_)
				{
					QString html;
					for (int i = 0, size = Impl_->ItemsFilterModel_->rowCount ();
							i < size; ++i)
					{
						QModelIndex index = Impl_->ItemsFilterModel_->index (i, 0);
						QModelIndex mapped = Impl_->ItemsFilterModel_->mapToSource (index);
						Item_ptr item = Core::Instance ().GetItem (mapped);
			
						html += ToHtml (item);
						html += "<hr />";
					}
			
					Impl_->Ui_.ItemView_->SetHtml (html);
				}
				else
				{
					QModelIndexList indexes = selection.indexes ();
			
					QModelIndex sindex;
					if (indexes.size ())
						sindex = Impl_->ItemsFilterModel_->mapToSource (indexes.at (0));
			
					if (!sindex.isValid () || indexes.size () != 2)
					{
						Impl_->Ui_.ItemView_->SetHtml ("");
						Impl_->ActionItemCommentsSubscribe_->setEnabled (false);
						return;
					}
			
					Core::Instance ().Selected (sindex);
			
					Item_ptr item = Core::Instance ().GetItem (sindex);
			
					Impl_->Ui_.ItemView_->SetHtml (ToHtml (item));
			
					QString commentsRSS = item->CommentsLink_;
					Impl_->ActionItemCommentsSubscribe_->setEnabled (!commentsRSS.isEmpty ());
				}
			}
			
			void ItemsWidget::makeCurrentItemVisible ()
			{
				QModelIndex item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
				if (item.isValid ())
					Impl_->Ui_.Items_->scrollTo (item);
			}
			
			void ItemsWidget::updateItemsFilter ()
			{
				int section = Impl_->Ui_.SearchType_->currentIndex ();
				QString text = Impl_->Ui_.SearchLine_->text ();
				switch (section)
				{
				case 1:
					Impl_->ItemsFilterModel_->setFilterWildcard (text);
					break;
				case 2:
					Impl_->ItemsFilterModel_->setFilterRegExp (text);
					break;
				default:
					Impl_->ItemsFilterModel_->setFilterFixedString (text);
					break;
				}
			}

			void ItemsWidget::selectorVisiblityChanged ()
			{
				if (!XmlSettingsManager::Instance ()->
						property ("ShowCategorySelector").toBool ())
				{
					Impl_->ItemCategorySelector_->selectAll ();
					Impl_->ItemCategorySelector_->hide ();
				}
				else if (Impl_->ItemCategorySelector_->GetSelections ().size ())
					Impl_->ItemCategorySelector_->show ();
			}
		};
	};
};

