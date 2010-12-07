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

#include "itemslistmodel.h"
#include <QApplication>
#include <QPalette>
#include <QTextDocument>
#include <QtDebug>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ItemsListModel::ItemsListModel (QObject *parent)
			: QAbstractItemModel (parent)
			, CurrentRow_ (-1)
			, CurrentChannel_ (-1)
			, MayBeRichText_ (false)
			{
				ItemHeaders_ << tr ("Name") << tr ("Date");
			}

			int ItemsListModel::GetSelectedRow () const
			{
				return CurrentRow_;
			}

			const IDType_t& ItemsListModel::GetCurrentChannel () const
			{
				return CurrentChannel_;
			}

			void ItemsListModel::SetCurrentChannel (const IDType_t& channel)
			{
				CurrentChannel_ = channel;
				reset ();
			}

			void ItemsListModel::Selected (const QModelIndex& index)
			{
				CurrentRow_ = index.row ();
				if (!index.isValid ())
					return;

				ItemShort item = CurrentItems_ [CurrentRow_];
				item.Unread_ = false;
				Core::Instance ().GetStorageBackend ()->UpdateItem (item);
			}

			void ItemsListModel::MarkItemReadStatus (const QModelIndex& i, bool read)
			{
				ItemShort is = CurrentItems_ [i.row ()];
				is.Unread_ = !read;
				Core::Instance ().GetStorageBackend ()->UpdateItem (is);
			}

			const ItemShort& ItemsListModel::GetItem (const QModelIndex& index) const
			{
				return CurrentItems_ [index.row ()];
			}

			bool ItemsListModel::IsItemRead (int item) const
			{
				return !CurrentItems_ [item].Unread_;
			}

			QStringList ItemsListModel::GetCategories (int item) const
			{
				return CurrentItems_ [item].Categories_;
			}

			void ItemsListModel::Reset (const IDType_t& channel)
			{
				CurrentChannel_ = channel;
				CurrentRow_ = -1;
				CurrentItems_.clear ();
				if (channel != static_cast<IDType_t> (-1))
				{
					Core::Instance ().GetStorageBackend ()->
						GetItems (CurrentItems_, channel);
					if (CurrentItems_.size ())
						MayBeRichText_ = Qt::mightBeRichText (CurrentItems_.at (0).Title_);
				}
				reset ();
			}

			namespace
			{
				struct FindEarlierDate
				{
					QDateTime Pattern_;

					FindEarlierDate (const QDateTime& pattern)
					: Pattern_ (pattern)
					{
					}

					bool operator() (const ItemShort& is)
					{
						return Pattern_ > is.PubDate_;
					}
				};
			};

			void ItemsListModel::ItemDataUpdated (Item_ptr item)
			{
				ItemShort is = item->ToShort ();

				items_shorts_t::iterator pos = CurrentItems_.end ();

				for (items_shorts_t::iterator i = CurrentItems_.begin (),
						end = CurrentItems_.end (); i != end; ++i)
					if (is.Title_ == i->Title_ &&
							is.URL_ == i->URL_)
					{
						pos = i;
						break;
					}

				// Item is new
				if (pos == CurrentItems_.end ())
				{
					items_shorts_t::iterator insertPos =
						std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
								FindEarlierDate (item->PubDate_));

					int shift = std::distance (CurrentItems_.begin (), insertPos);

					beginInsertRows (QModelIndex (), shift, shift);
					CurrentItems_.insert (insertPos, is);
					endInsertRows ();
				}
				// Item exists already
				else
				{
					*pos = is;

					int distance = std::distance (CurrentItems_.begin (), pos);
					emit dataChanged (index (distance, 0), index (distance, 1));
				}
			}

			int ItemsListModel::columnCount (const QModelIndex&) const
			{
				return ItemHeaders_.size ();
			}

			QVariant ItemsListModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid () || index.row () >= rowCount ())
					return QVariant ();

				if (role == Qt::DisplayRole)
				{
					switch (index.column ())
					{
						case 0:
							{
								QString title = CurrentItems_ [index.row ()].Title_;
								if (MayBeRichText_ &&
										Qt::mightBeRichText (title))
								{
									QTextDocument doc;
									doc.setHtml (title);
									title = doc.toPlainText ();
								}
								return title;
							}
						case 1:
							return CurrentItems_ [index.row ()].PubDate_;
						default:
							return QVariant ();
					}
				}
				//Color mark an items as read/unread
				else if (role == Qt::ForegroundRole)
				{
					bool palette = XmlSettingsManager::Instance ()->
							property ("UsePaletteColors").toBool ();
					if (CurrentItems_ [index.row ()].Unread_)
					{
						if (XmlSettingsManager::Instance ()->
								property ("UnreadCustomColor").toBool ())
							return XmlSettingsManager::Instance ()->
									property ("UnreadItemsColor").value<QColor> ();
						else
							return palette ?
								QApplication::palette ().link ().color () :
								QVariant ();
					}
					else
						return palette ?
							QApplication::palette ().linkVisited ().color () :
							QVariant ();
				}
				else if (role == Qt::FontRole &&
						CurrentItems_ [index.row ()].Unread_)
					return XmlSettingsManager::Instance ()->
						property ("UnreadItemsFont");
				else if (role == Qt::ToolTipRole)
				{
					IDType_t id = CurrentItems_ [index.row ()].ItemID_;
					Item_ptr item = Core::Instance ()
							.GetStorageBackend ()->GetItem (id);
					QString result = QString ("<qt><strong>%1</strong><br />").arg (item->Title_);
					if (item->Author_.size ())
					{
						result += tr ("<b>Author</b>: %1").arg (item->Author_);
						result += "<br />";
					}
					if (item->Categories_.size ())
					{
						result += tr ("<b>Categories</b>: %1").arg (item->Categories_.join ("; "));
						result += "<br />";
					}
					if (item->NumComments_ > 0)
					{
						result += tr ("%n comment(s)", "", item->NumComments_);
						result += "<br />";
					}
					if (item->Enclosures_.size () > 0)
					{
						result += tr ("%n enclosure(s)", "", item->Enclosures_.size ());
						result += "<br />";
					}
					if (item->MRSSEntries_.size () > 0)
					{
						result += tr ("%n MediaRSS entry(s)", "", item->MRSSEntries_.size ());
						result += "<br />";
					}
					if (item->CommentsLink_.size ())
					{
						result += tr ("RSS with comments is available");
						result += "<br />";
					}
					result += "<br />";
					const int maxDescriptionSize = 1000;
					result += item->Description_.left (maxDescriptionSize);
					if (item->Description_.size () > maxDescriptionSize)
						result += "...";
					return result;
				}
				else if (role == Qt::BackgroundRole)
				{
					const QPalette& p = QApplication::palette ();
					QLinearGradient grad (0, 0, 0, 10);
					grad.setColorAt (0, p.color (QPalette::AlternateBase));
					grad.setColorAt (1, p.color (QPalette::Base));
					return QBrush (grad);
				}
				else
					return QVariant ();
			}

			Qt::ItemFlags ItemsListModel::flags (const QModelIndex&) const
			{
				return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}

			QVariant ItemsListModel::headerData (int column, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return ItemHeaders_.at (column);
				else
					return QVariant ();
			}

			QModelIndex ItemsListModel::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();

				return createIndex (row, column);
			}

			QModelIndex ItemsListModel::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}

			int ItemsListModel::rowCount (const QModelIndex& parent) const
			{
				return parent.isValid () ? 0 : CurrentItems_.size ();
			}
		};
	};
};

