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

#include "feedsettings.h"
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			FeedSettings::FeedSettings (const QModelIndex& mapped, QWidget *parent)
			: QDialog (parent)
			, Index_ (mapped)
			{
				Ui_.setupUi (this);
			
				ChannelTagsCompleter_.reset (new Util::TagsCompleter (Ui_.ChannelTags_));
				Ui_.ChannelTags_->AddSelector ();
			
				connect (Ui_.ChannelLink_,
						SIGNAL (linkActivated (const QString&)),
						&Core::Instance (),
						SLOT (openLink (const QString&)));
			
				QStringList tags = Core::Instance ().GetTagsForIndex (Index_.row ());
				Ui_.ChannelTags_->setText (Core::Instance ().GetProxy ()->GetTagsManager ()->Join (tags));
			
				Feed::FeedSettings settings = Core::Instance ().GetFeedSettings (Index_);
				Ui_.UpdateInterval_->setValue (settings.UpdateTimeout_);
				Ui_.NumItems_->setValue (settings.NumItems_);
				Ui_.ItemAge_->setValue (settings.ItemAge_);
				Ui_.AutoDownloadEnclosures_->setChecked (settings.AutoDownloadEnclosures_);
			
				Core::ChannelInfo ci = Core::Instance ().GetChannelInfo (Index_);
				QString link = ci.Link_;
				QString shortLink;
				Ui_.ChannelLink_->setToolTip (link);
				if (link.size () >= 160)
					shortLink = link.left (78) + "..." + link.right (78);
				else
					shortLink = link;
				if (QUrl (link).isValid ())
				{
					link.insert (0, "<a href=\"");
					link.append ("\">" + shortLink + "</a>");
					Ui_.ChannelLink_->setText (link);
				}
				else
					Ui_.ChannelLink_->setText (shortLink);
			
				link = ci.URL_;
				Ui_.ChannelLink_->setToolTip (link);
				if (link.size () >= 160)
					shortLink = link.left (78) + "..." + link.right (78);
				else
					shortLink = link;
				if (QUrl (link).isValid ())
				{
					link.insert (0, "<a href=\"");
					link.append ("\">" + shortLink + "</a>");
					Ui_.FeedURL_->setText (link);
				}
				else
					Ui_.FeedURL_->setText (shortLink);
			
				Ui_.ChannelDescription_->setHtml (ci.Description_);
				Ui_.ChannelAuthor_->setText (ci.Author_);

				Ui_.FeedNumItems_->setText (QString::number (ci.NumItems_));
			
				QPixmap pixmap = Core::Instance ().GetChannelPixmap (Index_);
				if (pixmap.width () > 400)
					pixmap = pixmap.scaledToWidth (400, Qt::SmoothTransformation);
				if (pixmap.height () > 300)
					pixmap = pixmap.scaledToHeight (300, Qt::SmoothTransformation);
			}
			
			void FeedSettings::accept ()
			{
				QString tags = Ui_.ChannelTags_->text ();
				Core::Instance ().SetTagsForIndex (tags, Index_);
			
				Feed::FeedSettings settings (Ui_.UpdateInterval_->value (),
					Ui_.NumItems_->value (),
					Ui_.ItemAge_->value (),
					Ui_.AutoDownloadEnclosures_->checkState () == Qt::Checked);
				Core::Instance ().SetFeedSettings (settings, Index_);
			
				QDialog::accept ();
			}
			
			void FeedSettings::on_UpdateFavicon__released ()
			{
				Core::Instance ().UpdateFavicon (Index_);
			}
		};
	};
};

