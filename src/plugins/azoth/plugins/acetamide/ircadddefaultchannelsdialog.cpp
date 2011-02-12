/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircadddefaultchannelsdialog.h"
#include <QTableView>
#include <QPushButton>
#include <QMessageBox>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcAddDefaultChannelsDialog::IrcAddDefaultChannelsDialog (QWidget *parent)
	: QDialog (parent)
	, ChannelsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.DefaultChannels_->setModel (ChannelsModel_);
		QStringList headerLabels;
		headerLabels << tr ("Channel's name") << tr ("Password");
		ChannelsModel_->setHorizontalHeaderLabels (headerLabels);
		
		connect (Ui_.Add_,
				SIGNAL (clicked (bool)),
				this,
				SLOT (handleAddLine (bool)));
		
		connect (Ui_.Delete_,
				SIGNAL (clicked (bool)),
				this,
				SLOT (handleDeleteLine (bool)));
	}

	QStringList IrcAddDefaultChannelsDialog::GetChannels ()
	{
		QStringList channels; 
		for (int i = 0; i < ChannelsModel_->rowCount (); ++i)
			channels << ChannelsModel_->item (i)->text ();
		return channels;
	}

	QStringList IrcAddDefaultChannelsDialog::GetChannelsPair () const
	{
		QStringList channelsPair;
		for (int i = 0; i < ChannelsModel_->rowCount (); ++i)
		{
			channelsPair << ChannelsModel_->item (i, 0)->text () + 
									QString (" ") +
									ChannelsModel_->item (i, 1)->text ();
		}
		return channelsPair;
	}
	
	void IrcAddDefaultChannelsDialog::handleAddLine (bool checked)
	{
		QList<QStandardItem*> list;
		for (int i = 0; i < 2; ++i)
		{
			QStandardItem *item = new QStandardItem;
			item->setEditable (true);
			list << item;
		}
		ChannelsModel_->appendRow (list);
	}
	
	void IrcAddDefaultChannelsDialog::handleDeleteLine (bool checked)
	{
		QModelIndex index = Ui_.DefaultChannels_->currentIndex ();
		if (index.isValid ())
			ChannelsModel_->removeRow (index.row (), QModelIndex ());
	}
	
	void IrcAddDefaultChannelsDialog::accept ()
	{
		for (int i = 0; i < ChannelsModel_->rowCount (); ++i)
		{
			QString item = ChannelsModel_->item (i)->text ();
			if (item.isEmpty () ||
					item.contains (' ') ||
					item.contains (',') ||
					item.contains (QChar (7)))
			{
				QMessageBox::warning (this, 
						"LeechCraft",
						tr ("Invalid channel name"));
				return;
			}
		}
		
		QDialog::accept ();
	}
}
};
};
