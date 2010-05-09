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

#include "export.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include "feed.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Export::Export (const QString& title,
					const QString& exportTitle,
					const QString& choices,
					QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				setWindowTitle (title);
				Title_ = exportTitle;
				Choices_ = choices;
				Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (false);
				on_Browse__released ();
			}
			
			Export::~Export ()
			{
			}
			
			QString Export::GetDestination () const
			{
				return Ui_.File_->text ();
			}
			
			QString Export::GetTitle () const
			{
				return Ui_.Title_->text ();
			}
			
			QString Export::GetOwner () const
			{
				return Ui_.Owner_->text ();
			}
			
			QString Export::GetOwnerEmail () const
			{
				return Ui_.OwnerEmail_->text ();
			}
			
			std::vector<bool> Export::GetSelectedFeeds () const
			{
				std::vector<bool> result (Ui_.Channels_->topLevelItemCount ());
			
				for (int i = 0, items = Ui_.Channels_->topLevelItemCount ();
						i < items; ++i)
					result [i] = (Ui_.Channels_->topLevelItem (i)->
							data (0, Qt::CheckStateRole) == Qt::Checked);
			
				return result;
			}
			
			void Export::SetFeeds (const channels_shorts_t& channels)
			{
				for (channels_shorts_t::const_iterator i = channels.begin (),
						end = channels.end (); i != end; ++i)
				{
					QStringList strings;
					Feed_ptr feed = Core::Instance ().GetStorageBackend ()->
							GetFeed (i->FeedID_);
					strings << i->Title_ << feed->URL_;
			
					QTreeWidgetItem *item =
						new QTreeWidgetItem (Ui_.Channels_, strings);
					item->setData (0, Qt::CheckStateRole, Qt::Checked);
				}
			}
			
			void Export::on_File__textEdited (const QString& text)
			{
				Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (!text.isEmpty ());
			}
			
			void Export::on_Browse__released ()
			{
				QString startingPath = QFileInfo (Ui_.File_->text ()).path ();
				if (Ui_.File_->text ().isEmpty () ||
						startingPath.isEmpty ())
					startingPath = QDir::homePath () + "/feeds.opml";
			
				QString filename = QFileDialog::getSaveFileName (this,
						Title_,
						startingPath,
						Choices_);
				if (filename.isEmpty ())
				{
					QTimer::singleShot (0,
							this,
							SLOT (reject ()));
					return;
				}
			
				Ui_.File_->setText (filename);
				Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (true);
			}
		};
	};
};

