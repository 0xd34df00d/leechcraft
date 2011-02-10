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

#include "ircaccountconfigurationdialog.h"
#include <boost/shared_ptr.hpp>
#include <QComboBox>
#include <QtDebug>
#include <QMap>
#include <QMenu>
#include "ircaddserverdialog.h"
#include "ircadddefaultchannelsdialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcAccountConfigurationDialog::IrcAccountConfigurationDialog (QWidget *parent)
	: QDialog (parent)
	, LastIndex_ (0)
	, AddMenu_ (new QMenu (this))
	, AddChannel_ (new QAction (tr ("Add channel"), AddMenu_))
	, AddServer_ (new QAction (tr ("Add server"), AddMenu_))
	, ServerAndChannels_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		
		AddChannel_->setEnabled (false);
		AddMenu_->addAction (AddServer_);
		AddMenu_->addAction (AddChannel_);
		Ui_.Add_->setMenu (AddMenu_);

		Ui_.ServerChannels_->setModel (ServerAndChannels_);
		
		connect (Ui_.Servers_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleChangeServer (int)));
		
		connect (AddServer_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleAddServer (bool)));
		
		connect (AddChannel_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleAddChannel (bool)));
		
		connect (Ui_.ServerChannels_,
				SIGNAL (clicked (QModelIndex)),
				this,
				SLOT (handleChannelsEnable (QModelIndex)));
	}
	
	QMap<QString, QVariant> IrcAccountConfigurationDialog::GetNicks ()
	{
		Nicknames_ [Ui_.Servers_->currentText ()].toStringList () = 
				Ui_.NickNames_->toPlainText ().split ('\n');
		return Nicknames_;
	}

	void IrcAccountConfigurationDialog::SetNicks (const QMap<QString, QVariant>& nicks)
	{
		Nicknames_ = nicks;
		SetNetworks (Nicknames_.keys ());
	}
	
	QMap<QString, QVariant> IrcAccountConfigurationDialog::GetServers () const
	{
		return Servers_;
	}
	
	void IrcAccountConfigurationDialog::SetServers(const QMap<QString, QVariant>& servers)
	{
		Q_FOREACH (const QString& network, servers.keys ())
		{
			QStandardItem *networkItem = new QStandardItem (network);
			networkItem->setData ("network", ServerAndChannelsRole);
			networkItem->setEditable (false);
			QList<QStandardItem*> serversList;
			Q_FOREACH (const QString& server, servers [network].toStringList ())
			{
				QStandardItem *serverItem = new QStandardItem (server);
				serverItem->setData ("server", ServerAndChannelsRole);
				serverItem->setEditable (false);
				serversList << serverItem;
			}
			networkItem->appendRows (serversList);
			
			ServerAndChannels_->appendRow (networkItem);
		}
	}

	QMap<QString, QVariant> IrcAccountConfigurationDialog::GetChannels () const
	{
		return Channels_;
	}

	void IrcAccountConfigurationDialog::GetServersAndChannels (QStandardItem *item, bool first)
	{
		if (first)
			GetServersAndChannels (ServerAndChannels_->item (0), false);
		else
		{
			if (item->data (ServerAndChannelsRole).toString () == "server")
				Servers_ [item->parent ()->text ()] = 
						Servers_ [item->parent ()->text ()].toStringList () 
								<< item->text ();
			else if (item->data (ServerAndChannelsRole).toString () == "channel")
				Channels_ [item->parent ()->text ()] = 
						Channels_ [item->parent ()->text ()].toStringList () 
								<< item->text ();
								
			for (int i = 0; i < item->rowCount (); ++i)
				GetServersAndChannels (item->child (i), false);
		}
	}
	
	QStringList IrcAccountConfigurationDialog::GetNetworks () const
	{
		QStringList result;
		for (int i = 0; i < Ui_.Servers_->count (); i++)
			result << Ui_.Servers_->itemText (i);
		return result;
	}

	void IrcAccountConfigurationDialog::SetNetworks (const QStringList& networks)
	{
		Ui_.Servers_->addItems (networks);
		Ui_.Networks_->addItems (networks);
	}
	
	void IrcAccountConfigurationDialog::handleChangeServer (int index)
	{
		Nicknames_ [Ui_.Servers_->itemText (LastIndex_)].toStringList () = 
				Ui_.NickNames_->toPlainText ().split ('\n');
		Ui_.NickNames_->setPlainText (Nicknames_ [Ui_.Servers_->currentText ()]
											.toStringList ().join ("\n"));
		LastIndex_ = index;
	}
	
	void IrcAccountConfigurationDialog::handleAddServer (bool checked)
	{
		std::auto_ptr<IrcAddServerDialog> dsa (new IrcAddServerDialog (0));
		
		if (dsa->exec () == QDialog::Rejected)
			return;
		
		QString network = Ui_.Networks_->currentText ();
		QList<QStandardItem*> networkList = ServerAndChannels_->findItems (network);
		
		if (networkList.isEmpty ())
		{
			QStandardItem *item = new QStandardItem (network);
			item->setData ("network", ServerAndChannelsRole);
			item->setEditable (false);
			networkList.push_back (item);
			ServerAndChannels_->appendRow (networkList.first ());
		}
		QStandardItem *item = new QStandardItem (dsa->GetServer ());
		item->setData ("server", ServerAndChannelsRole);
		item->setEditable (false);
		networkList.first ()->appendRow (item);
	}

	void IrcAccountConfigurationDialog::handleAddChannel (bool checked)
	{
		std::auto_ptr<IrcAddDefaultChannelsDialog> dac (new IrcAddDefaultChannelsDialog (0));
		
		if (dac->exec () == QDialog::Rejected)
			return;
		
		QModelIndex currentIndex = Ui_.ServerChannels_->currentIndex (); 
		QModelIndex serverIndex = currentIndex;
		if (currentIndex.data (ServerAndChannelsRole).toString () == "channel")
			serverIndex = currentIndex.parent ();
		
		QList<QStandardItem*> channelsList;
		Q_FOREACH (const QString& channel, dac->GetChannels ())
		{
			QStandardItem *channelItem = new QStandardItem (channel);
			channelItem->setData ("channel", ServerAndChannelsRole);
			channelItem->setEditable (false);
			channelsList.push_back (channelItem);
		}
		
		ServerAndChannels_->itemFromIndex (serverIndex)->appendRows (channelsList);
	}

	void IrcAccountConfigurationDialog::handleChannelsEnable (QModelIndex index)
	{
		AddChannel_->setEnabled (!(index.data (ServerAndChannelsRole) != "server" && 
				index.data (ServerAndChannelsRole) != "channel"));
	}



};
};
};