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
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QMap>
#include <QMenu>
#include <QLineEdit>
#include <QTabWidget>
#include "ircaddserverdialog.h"
#include "ircadddefaultchannelsdialog.h"
#include "irceditchanneldialog.h"

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
		
// 		if (Nicknames_.isEmpty ())
// 			SetServers (QStringList ());

// 		connect (Ui_.Servers_,
// 				SIGNAL (currentIndexChanged (int)),
// 				this,
// 				SLOT (handleChangeServer (int)));
// 		
// 		connect (AddServer_,
// 				SIGNAL (triggered (bool)),
// 				this,
// 				SLOT (handleAddServer (bool)));
// 		
// 		connect (AddChannel_,
// 				SIGNAL (triggered (bool)),
// 				this,
// 				SLOT (handleAddChannel (bool)));
// 		
// 		connect (Ui_.Edit_,
// 				SIGNAL (clicked (bool)),
// 				this,
// 				SLOT (handleEditElement (bool)));
// 		
// 		connect (Ui_.Delete_,
// 				SIGNAL (clicked (bool)),
// 				this,
// 				SLOT (handleDeleteElement (bool)));
// 		
// 		connect (Ui_.ServerChannels_,
// 				SIGNAL (clicked (QModelIndex)),
// 				this,
// 				SLOT (handleChannelsEnable (QModelIndex)));
// 		
// 		connect (Ui_.Networks_,
// 				SIGNAL (textChanged (const QString&)),
// 				this,
// 				SLOT (handleNetworkTextChange (const QString&)));
// 		
// 		connect (Ui_.ConfigurationWidget_,
// 				SIGNAL (currentChanged (int)),
// 				this,
// 				SLOT (handleTabChange (int)));
// 		
// 		connect (Ui_.ServerChannels_,
// 				SIGNAL (doubleClicked (const QModelIndex&)),
// 				this,
// 				SLOT (handleDblClick (const QModelIndex&)));
	}
	
	QString IrcAccountConfigurationDialog::GetDefaultNickname () const
	{
// 		Q_FOREACH (const NickNameData& nick, Nicknames_)
// 			if (nick.Server_ == Ui_.Servers_->itemData (0, DefaultServerRole))
// 				return nick.Nicks_.at (0);
// 		return QString ();
	}

	QList<ServerOptions> IrcAccountConfigurationDialog::GetServers () const
	{
		return Servers_;
// 		bool found = false;
// 		QVariant server = Ui_.Servers_->itemData (Ui_.Servers_->currentIndex (), DefaultServerRole);
// 		for (int i = 0, size = Nicknames_.count (); i < size; ++i)
// 			if (Nicknames_.at (i).Server_ == server.toString ())
// 			{
// 				Nicknames_ [i] = GetNicknameData (server.toString (), Ui_.Servers_->currentText ());
// 				found = true;
// 				break;
// 			}
// 
// 		if (!found && !Ui_.NickNames_->toPlainText ().isEmpty ())
// 			Nicknames_ << GetNicknameData (server.toString (), Ui_.Servers_->currentText ());
// 
// 		return Nicknames_;
	}

	void IrcAccountConfigurationDialog::SetServers (const QList<ServerOptions> & servers)
	{
		Servers_ = servers;
// 		Nicknames_ = nicks;
// 		QStringList servers;
// 		Q_FOREACH (const NickNameData& item, Nicknames_)
// 			if (!item.ServerName_.isEmpty ())
// 				servers << item.ServerName_;
// 
// 		servers.removeDuplicates ();
// 
// 		SetServers (servers);
	}
	
	void IrcAccountConfigurationDialog::SetChannels (const QList<ChannelOptions>& channels)
	{
		Channels_ = channels;
	}

	QList<ChannelOptions> IrcAccountConfigurationDialog::GetChannels () const
	{
		return Channels_;
	}


/*	QList<ServerInfoData> IrcAccountConfigurationDialog::GetServersInfo () const
	{
// 		return ServersInfo_;
	}

	void IrcAccountConfigurationDialog::SetDefaultServers (const QList<ServerInfoData>& serversInfo)
	{
// 		ServerAndChannels_->clear ();
// 
// 		Q_FOREACH (const ServerInfoData& serverInfo, serversInfo)
// 		{
// 			QString networkName = serverInfo.Network_;
// 			QList<QStandardItem*> networks = ServerAndChannels_->findItems (networkName);
// 			QStandardItem *networkItem;
// 
// 			if (networks.isEmpty ())
// 			{
// 				networkItem = new QStandardItem (networkName);
// 				networkItem->setData ("network", ServerAndChannelsRole);
// 				ServerAndChannels_->appendRow (networkItem);
// 			}
// 			else
// 				networkItem = networks.at (0);
// 
// 			QList<QStandardItem*> channels;
// 			Q_FOREACH (const QString& channelPair, serverInfo.Channels_)
// 			{
// 				QStandardItem *item = new QStandardItem (channelPair.split (' ').at (0));
// 				item->setData ("channel", ServerAndChannelsRole);
// 				item->setEditable (false);
// 				channels << item;
// 			}
// 
// 			QStandardItem *serverItem = new QStandardItem (serverInfo.Server_);
// 			serverItem->setData ("server", ServerAndChannelsRole);
// 			serverItem->setEditable (false);
// 			serverItem->appendRows (channels);
// 			networkItem->appendRow (serverItem);
// 		}
	}

// 	QStringList IrcAccountConfigurationDialog::GetServers () const
// 	{
// 		QStringList result;
// 		for (int i = 1, size = Ui_.Servers_->count (); i < size; i++)
// 			result << Ui_.Servers_->itemText (i);
// 		return result;
// 	}

// 	void IrcAccountConfigurationDialog::SetServers (const QStringList& servers)
// 	{
// 		Ui_.Servers_->clear ();
// 		Ui_.Servers_->addItems (servers);
// 		
// 		if (servers.isEmpty ())
// 		{
// 			Ui_.Servers_->addItem (tr ("Default"));
// 			Ui_.Servers_->setItemData (0, Ui_.Servers_->itemText (0).toLower (), DefaultServerRole);
// 			return;
// 		}
// 		
// 		for (int i = 0, size = Ui_.Servers_->count(); i < size; i++)
// 		{
// 			Ui_.Servers_->setItemData (i, Ui_.Servers_->itemText (i).toLower (), DefaultServerRole);
// 			if (Ui_.Servers_->itemData (Ui_.Servers_->currentIndex (), DefaultServerRole).toString () ==
// 					Nicknames_.at (i).Server_)
// 			{
// 				Ui_.NickNames_->setPlainText (Nicknames_.at (i).Nicks_.join ("\n"));
// 				Ui_.GenerateNicknames_->setChecked (Nicknames_.at (i).AutoGenerate_);
// 			}
// 		}
// 	}
	
	void IrcAccountConfigurationDialog::DeleteNetwork (const QModelIndex& index, const QString& message)
	{
// 		int ret = QMessageBox::warning (this, "LeechCraft",
// 				message,
// 				QMessageBox::Ok | QMessageBox::Cancel,
// 				QMessageBox::Cancel);
// 
// 		if (ret != QMessageBox::Ok)
// 			return;
// 
// 		for (int i = ServersInfo_.count () - 1; i >= 0; --i)
// 			if (ServersInfo_.at (i).Network_ == index.data ())
// 				ServersInfo_.removeAt (i);
// 
// 		SetServersInfo (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::DeleteServer (const QModelIndex& index, const QString& message)
	{
// 		int ret = QMessageBox::warning (this, "LeechCraft",
// 				message,
// 				QMessageBox::Ok | QMessageBox::Cancel,
// 				QMessageBox::Cancel);
// 
// 		if (ret != QMessageBox::Ok)
// 			return;
// 
// 		for (int i = ServersInfo_.count () - 1; i >= 0; --i)
// 			if (ServersInfo_.at (i).Server_ == index.data ())
// 				ServersInfo_.removeAt (i);
// 
// 		SetServersInfo (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::DeleteChannel (const QModelIndex& index, 
			const QString& message)
	{
// 		int ret = QMessageBox::warning (this, "LeechCraft",
// 				message,
// 				QMessageBox::Ok | QMessageBox::Cancel,
// 				QMessageBox::Cancel);
// 
// 		if (ret != QMessageBox::Ok)
// 			return;
// 
// 		for (int i = ServersInfo_.count () - 1; i >= 0; --i)
// 		{
// 			if (ServersInfo_.at (i).Server_ == index.parent ().data ().toString () &&
// 					ServersInfo_.at (i).Network_ == index.parent ().parent ().data ().toString ())
// 			{
// 				QStringList list = ServersInfo_.at (i).Channels_;
// 
// 				for (int k = list.count () - 1; k >= 0; --k)
// 				{
// 					QString str = list.at (k);
// 					if (str.split (' ').at (0) == index.data ().toString ())
// 					{
// 						list.removeAt (k);
// 						break;
// 					}
// 				}
// 				ServersInfo_ [i].Channels_ = list;
// 			}
// 		}
// 		
// 		SetServersInfo (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::EditChannel ()
	{
// 		std::auto_ptr<IrcEditChannelDialog> dec (new IrcEditChannelDialog (0));
// 		
// 		QVariant server = Ui_.ServerChannels_->currentIndex ().parent ().data ();
// 		QVariant network = Ui_.ServerChannels_->currentIndex ().parent ().parent ().data ();
// 		QVariant channel = Ui_.ServerChannels_->currentIndex ().data ();
// 
// 		dec->SetChannel (channel.toString ());
// 		dec->SetPassword (GetChannelPassword (server.toString (),
// 				network.toString (),
// 				channel.toString ()));
// 
// 		if (dec->exec () == QDialog::Rejected)
// 			return;
// 
// 		for (int i = 0, size = ServersInfo_.count (); i < size; ++i)
// 		{
// 			ServerInfoData serverInfo = ServersInfo_.at (i);
// 			if (serverInfo.Network_ ==  network &&
// 					serverInfo.Server_ == server)
// 			{
// 				QStringList channels = serverInfo.Channels_;
// 				for (int j = 0, chsize = channels.count (); j < chsize; ++j)
// 					if (channels [j].split (' ').at (0) == channel.toString ())
// 					{
// 						channels [j] = dec->GetChannel () + QString (" ") + dec->GetPassword ();
// 						break;
// 					}
// 				serverInfo.Channels_ = channels;
// 				ServersInfo_ [i] = serverInfo;
// 			}
// 		}
// 
// 		SetDefaultServers (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::EditServer ()
	{
// 		std::auto_ptr<IrcAddServerDialog> dsa (new IrcAddServerDialog (0));
// 
// 		QStringList channels;
// 		QModelIndex currentIndex = Ui_.ServerChannels_->currentIndex ();
// 		Q_FOREACH (const ServerInfoData& serverInfo, ServersInfo_)
// 			if (serverInfo.Server_ == currentIndex.data () &&
// 					serverInfo.Network_ == currentIndex.parent ().data ())
// 			{
// 				dsa->SetServer (serverInfo.Server_);
// 				dsa->SetPassword (serverInfo.Password_);
// 				dsa->SetPort (serverInfo.Port_);
// 				dsa->SetSSL (serverInfo.SSL_);
// 				channels = serverInfo.Channels_;
// 			}
// 
// 		if (dsa->exec () == QDialog::Rejected)
// 			return;
// 
// 		ServerInfoData server;
// 		server.Network_ = Ui_.Networks_->text ();
// 		server.Server_ = dsa->GetServer ();
// 		server.Port_ = dsa->GetPort ();
// 		server.Password_ = dsa->GetPassword ();
// 		server.SSL_ = dsa->GetSSL ();
// 		server.Channels_ = channels;
// 
// 		for (int i = 0, size = ServersInfo_.count (); i < size; ++i)
// 			if (ServersInfo_.at (i).Server_ == currentIndex.data () &&
// 					ServersInfo_.at (i).Network_ == currentIndex.parent ().data ())
// 			{
// 				ServersInfo_ [i] = server;
// 				break;
// 			}
// 
// 		SetDefaultServers (ServersInfo_);
	}

	QString IrcAccountConfigurationDialog::GetChannelPassword (const QString& server, 
			const QString& network, const QString& channel)
	{
// 		Q_FOREACH (const ServerInfoData& serverInfo, ServersInfo_)
// 			if (serverInfo.Network_ == network &&
// 					serverInfo.Server_ == server)
// 			{
// 				QStringList channels = serverInfo.Channels_;
// 				Q_FOREACH (const QString& chnnl, channels)
// 					if (chnnl.split (' ').at (0) == channel)
// 						if (chnnl.split (' ').count () > 1)
// 							return chnnl.split (' ').at (1);
// 			}
// 
// 		return QString ();
	}

	NickNameData IrcAccountConfigurationDialog::GetNicknameData (const QString& server, const QString& name)
	{
// 		NickNameData map;
// 		map.Nicks_ = Ui_.NickNames_->toPlainText ().split ('\n');
// 		map.AutoGenerate_ = Ui_.GenerateNicknames_->isChecked ();
// 		map.Server_ = server;
// 		map.ServerName_ = name;
// 
// 		return map;
	}

	bool IrcAccountConfigurationDialog::IsServerExists (const ServerInfoData& server)
	{
// 		Q_FOREACH (const ServerInfoData& serverInfo, ServersInfo_)
// 		{
// 			if (serverInfo.Network_ == server.Network_ &&
// 					serverInfo.Server_ == server.Server_ &&
// 					serverInfo.Port_ == server.Port_ &&
// 					serverInfo.SSL_ == server.SSL_)
// 				return true;
// 		}
// 		
// 		return false;
	}

	QStringList IrcAccountConfigurationDialog::RemoveDuplicatesChannels (const QString& network, 
			const QString& server, const QStringList& channels)
	{
// 		QStringList strList = channels;
// 		
// 		Q_FOREACH (const ServerInfoData& serverInfo, ServersInfo_)
// 		{
// 			if (serverInfo.Network_ == network &&
// 					serverInfo.Server_ == server)
// 			{
// 				Q_FOREACH (const QString& str, serverInfo.Channels_)
// 				{
// 					for (int i = 0, size = strList.count (); i < size; ++i)
// 					{
// 						if (str.split (' ', QString::SkipEmptyParts).at (0) == 
// 								strList.at (i).split (' ', QString::SkipEmptyParts).at (0))
// 						{
// 							strList.removeAt (i);
// 							break;
// 						}
// 					}
// 				}
// 			}
// 		}
// 		
// 		return strList;
	}

	void IrcAccountConfigurationDialog::handleChangeServer (int index)
	{
// 		bool found = false;
// 		QVariant lastServer = Ui_.Servers_->itemData (LastIndex_, DefaultServerRole);
// 
// 		for (int i = 0, size = Nicknames_.count (); i < size; ++i)
// 			if (Nicknames_.at (i).Server_ == lastServer)
// 			{
// 				Nicknames_ [i] = GetNicknameData (lastServer.toString (), Ui_.Servers_->itemText (LastIndex_));
// 				found = true;
// 				break;
// 			}
// 
// 		if (!found && !Ui_.NickNames_->toPlainText ().isEmpty ())
// 			Nicknames_ << GetNicknameData (lastServer.toString (), Ui_.Servers_->itemText (LastIndex_));
// 
// 		Ui_.NickNames_->setPlainText (QString ());
// 
// 		Q_FOREACH (const NickNameData& item, Nicknames_)
// 		{
// 			if (item.Server_ == Ui_.Servers_->itemData (Ui_.Servers_->currentIndex (), DefaultServerRole))
// 			{
// 				Ui_.NickNames_->setPlainText (item.Nicks_.join ("\n"));
// 				Ui_.GenerateNicknames_->setChecked (item.AutoGenerate_);
// 			}
// 			LastIndex_ = Ui_.Servers_->currentIndex ();
// 		}
	}

	void IrcAccountConfigurationDialog::handleAddServer (bool checked)
	{
// 		std::auto_ptr<IrcAddServerDialog> dsa (new IrcAddServerDialog (0));
// 		
// 		if (dsa->exec () == QDialog::Rejected)
// 			return;
// 		
// 		ServerInfoData server;
// 		server.Network_ = Ui_.Networks_->text ();
// 		server.Server_ = dsa->GetServer ();
// 		server.Port_ = dsa->GetPort ();
// 		server.Password_ = dsa->GetPassword ();
// 		server.SSL_ = dsa->GetSSL ();
// 		
// 		if (IsServerExists (server))
// 			return;
// 		
// 		ServersInfo_ << server;
// 		
// 		NickNameData nick;
// 		nick.AutoGenerate_ = false;
// 		nick.ServerName_ = dsa->GetServer ();
// 		nick.Server_ = dsa->GetServer ().toLower ();
// 		nick.Nicks_ = QStringList ();
// 		Nicknames_ << nick;
// 		
// 		QString network = Ui_.Networks_->text ();
// 		QList<QStandardItem*> networkList = ServerAndChannels_->findItems (network);
// 		
// 		if (networkList.isEmpty ())
// 		{
// 			QStandardItem *item = new QStandardItem (network);
// 			item->setData ("network", ServerAndChannelsRole);
// 			networkList.push_back (item);
// 			ServerAndChannels_->appendRow (networkList.first ());
// 		}
// 		QStandardItem *item = new QStandardItem (dsa->GetServer ());
// 		item->setData ("server", ServerAndChannelsRole);
// 		item->setEditable (false);
// 		networkList.first ()->appendRow (item);
	}

	void IrcAccountConfigurationDialog::handleAddChannel (bool checked)
	{
// 		std::auto_ptr<IrcAddDefaultChannelsDialog> dac (new IrcAddDefaultChannelsDialog (0));
// 		
// 		QModelIndex currentIndex = Ui_.ServerChannels_->currentIndex (); 
// 		QModelIndex serverIndex = currentIndex;
// 		
// 		if (currentIndex.data (ServerAndChannelsRole).toString () == "channel")
// 			serverIndex = currentIndex.parent ();
// 		
// 		QVariant server = serverIndex.data ().toString ();
// 		QVariant network = serverIndex.parent ().data ().toString ();
// 		
// 		if (dac->exec () == QDialog::Rejected)
// 			return;
// 
// 		QStringList list = RemoveDuplicatesChannels (network.toString (),
// 				server.toString (), dac->GetChannels ());
// 		
// 		if (list.isEmpty ())
// 			return;
// 		
// 		for (int i = 0, size = ServersInfo_.count (); i < size; i++)
// 		{
// 			ServerInfoData srv = ServersInfo_.at (i);
// 			if (srv.Server_ == server && 
// 					srv.Network_ == network)
// 				srv.Channels_ +=RemoveDuplicatesChannels (network.toString (),
// 										server.toString (), dac->GetChannelsPair ());
// 			ServersInfo_ [i] = srv;
// 		}
// 
// 		QList<QStandardItem*> channelsList;
// 		Q_FOREACH (const QString& channel, list)
// 		{
// 			QStandardItem *channelItem = new QStandardItem (channel);
// 			channelItem->setData ("channel", ServerAndChannelsRole);
// 			channelItem->setEditable (false);
// 			channelsList.push_back (channelItem);
// 		}
// 		
// 		ServerAndChannels_->itemFromIndex (serverIndex)->appendRows (channelsList);
	}

	void IrcAccountConfigurationDialog::handleChannelsEnable (QModelIndex index)
	{
// 		AddChannel_->setEnabled (!(index.data (ServerAndChannelsRole) != "server" && 
// 				index.data (ServerAndChannelsRole) != "channel"));
	}
	
	void IrcAccountConfigurationDialog::handleNetworkTextChange (const QString& text)
	{
// 		AddServer_->setEnabled (!text.isEmpty ());
	}
	
	void IrcAccountConfigurationDialog::handleEditElement (bool checked)
	{
// 		QModelIndex index = Ui_.ServerChannels_->currentIndex ();
// 		if (index.data (ServerAndChannelsRole).toString () == "channel")
// 			EditChannel ();
// 		else if (index.data (ServerAndChannelsRole).toString () == "server")
// 			EditServer ();
// 		else if (index.data (ServerAndChannelsRole).toString () == "network")
// 			Ui_.ServerChannels_->edit (index);
	}

	void IrcAccountConfigurationDialog::handleDeleteElement (bool checked)
	{
// 		if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "network")
// 			DeleteNetwork (Ui_.ServerChannels_->currentIndex (),
// 					tr ("All servers and channels of this network will be delete too.\nAre you sure?"));
// 		else if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "server")
// 			DeleteServer (Ui_.ServerChannels_->currentIndex (),
// 					tr ("All channels of this server will be delete too.\nAre you sure?"));
// 		else if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "channel")
// 			DeleteChannel (Ui_.ServerChannels_->currentIndex (),
// 					tr ("Are you really want to delete this channel?"));
	}

	void IrcAccountConfigurationDialog::handleTabChange (int index)
	{
// 		if (!index)
// 		{
// 			QStringList str;
// 			Q_FOREACH (const NickNameData& nick, Nicknames_)
// 				if (!nick.ServerName_.isEmpty ())
// 					str << nick.ServerName_;
// 			
// 			str.removeDuplicates ();
// 			SetServers (str);
// 		}
// 		else
// 		{
// 			bool found = false;
// 			QVariant server = Ui_.Servers_->itemData (Ui_.Servers_->currentIndex (), DefaultServerRole);
// 			for (int i = 0, size = Nicknames_.count (); i < size; ++i)
// 				if (Nicknames_.at (i).Server_ == server.toString ())
// 				{
// 					Nicknames_ [i] = GetNicknameData (server.toString (), Ui_.Servers_->currentText ());
// 					found = true;
// 				}
// 				
// 			if (!found && !Ui_.NickNames_->toPlainText ().isEmpty ())
// 				Nicknames_ << GetNicknameData (server.toString (), Ui_.Servers_->currentText ());
// 		}
	}
	
	void IrcAccountConfigurationDialog::handleDblClick (const QModelIndex& index)
	{
// 		if (index.isValid ())
// 			handleEditElement (false);
	}*/
};
};
};
