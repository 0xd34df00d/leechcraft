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
		
		connect (Ui_.Edit_,
				SIGNAL (clicked (bool)),
				this,
				SLOT (handleEditElement (bool)));
		
		connect (Ui_.Delete_,
				SIGNAL (clicked (bool)),
				this,
				SLOT (handleDeleteElement (bool)));
		
		connect (Ui_.ServerChannels_,
				SIGNAL (clicked (QModelIndex)),
				this,
				SLOT (handleChannelsEnable (QModelIndex)));
		
		connect (Ui_.Networks_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (handleNetworkTextChange (const QString&)));
		
		connect (Ui_.ConfigurationWidget_,
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleTabChange (int)));
		
		connect (Ui_.ServerChannels_,
				SIGNAL (doubleClicked (const QModelIndex&)),
				this,
				SLOT (handleDblClick (const QModelIndex&)));
	}
	
	QList<QVariant> IrcAccountConfigurationDialog::GetNicks ()
	{
		bool found = false;
		QString network = Ui_.Servers_->currentText ();
		for (int i = 0; i < Nicknames_.count (); ++i)
			if (Nicknames_.at (i).toMap () ["Network"].toString () == network)
			{
				Nicknames_ [i] = GetNicknameData (network);
				found = true;
				break;
			}

		if (!found)
			Nicknames_ << GetNicknameData (network);

		return Nicknames_;
	}

	void IrcAccountConfigurationDialog::SetNicks (const QList<QVariant> & nicks)
	{
		Nicknames_ = nicks;
		QStringList networks;
		Q_FOREACH (const QVariant& item, Nicknames_)
			networks << item.toMap () ["Network"].toString ();

		SetNetworks (networks);
	}

	void IrcAccountConfigurationDialog::SetServers (const QList<QVariant>& serversInfo)
	{
		ServerAndChannels_->clear ();

		Q_FOREACH (const QVariant& serverInfo, serversInfo)
		{
			QString networkName = serverInfo.toMap () ["Network"].toString ();
			QList<QStandardItem*> networks = ServerAndChannels_->findItems (networkName);
			QStandardItem *networkItem;

			if (networks.isEmpty ())
			{
				networkItem = new QStandardItem (networkName);
				networkItem->setData ("network", ServerAndChannelsRole);
				ServerAndChannels_->appendRow (networkItem);
			}
			else
				networkItem = networks.at (0);

			QList<QStandardItem*> channels;
			Q_FOREACH (const QString& channelPair, serverInfo.toMap () ["Channels"].toStringList ())
			{
				QStandardItem *item = new QStandardItem (channelPair.split (' ').at (0));
				item->setData ("channel", ServerAndChannelsRole);
				item->setEditable (false);
				channels << item;
			}

			QStandardItem *serverItem = new QStandardItem (serverInfo.toMap () ["Server"].toString ());
			serverItem->setData ("server", ServerAndChannelsRole);
			serverItem->setEditable (false);
			serverItem->appendRows (channels);
			networkItem->appendRow (serverItem);
		}
	}

	QList<QVariant> IrcAccountConfigurationDialog::GetServersInfo () const
	{
		return ServersInfo_;
	}

	void IrcAccountConfigurationDialog::SetServersInfo (const QList<QVariant>& serversInfo)
	{
		ServersInfo_ = serversInfo;
		SetServers (ServersInfo_);

		QStringList networks;
		Q_FOREACH (const QVariant& serverInfo, ServersInfo_)
			networks << serverInfo.toMap () ["Network"].toString ();;

		networks.removeDuplicates ();
		SetNetworks (networks);
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
		Ui_.Servers_->clear ();
		Ui_.Servers_->addItems (networks);
	}
	
	void IrcAccountConfigurationDialog::DeleteElement (const QString& key, 
			const QModelIndex& index, const QString& message)
	{
		int ret = QMessageBox::warning (this, "LeechCraft",
				message,
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel);

		if (ret != QMessageBox::Ok)
			return;

		for (int i = ServersInfo_.count () - 1; i >= 0; --i)
			if (ServersInfo_.at (i).toMap () [key] == index.data ())
				ServersInfo_.removeAt (i);

		SetServersInfo (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::DeleteChannel (const QModelIndex& index, 
			const QString& message)
	{
		int ret = QMessageBox::warning (this, "LeechCraft",
				message,
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel);

		if (ret != QMessageBox::Ok)
			return;

		for (int i = ServersInfo_.count () - 1; i >= 0; --i)
		{
			if (ServersInfo_.at (i).toMap () ["Server"] == index.parent ().data () &&
					ServersInfo_.at (i).toMap () ["Network"] == index.parent ().parent ().data ())
			{
				QStringList list = ServersInfo_.at (i).toMap () ["Channels"].toStringList ();

				for (int k = list.count () - 1; k >= 0; --k)
				{
					QString str = list.at (k);
					if (str.split (' ').at (0) == index.data ().toString ())
					{
						list.removeAt (k);
						break;
					}
				}
				
				QMap<QString, QVariant> map = ServersInfo_.at (i).toMap ();
				map ["Channels"] = list;
				ServersInfo_ [i] = map;
			}
		}
		
		SetServersInfo (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::EditChannel ()
	{
		std::auto_ptr<IrcEditChannelDialog> dec (new IrcEditChannelDialog (0));
		
		QVariant server = Ui_.ServerChannels_->currentIndex ().parent ().data ();
		QVariant network = Ui_.ServerChannels_->currentIndex ().parent ().parent ().data ();
		QVariant channel = Ui_.ServerChannels_->currentIndex ().data ();

		dec->SetChannel (channel.toString ());
		dec->SetPassword (GetChannelPassword (server.toString (),
				network.toString (),
				channel.toString ()));

		if (dec->exec () == QDialog::Rejected)
			return;

		for (int i = 0; i < ServersInfo_.count (); ++i)
		{
			QMap<QString, QVariant> serverInfo = ServersInfo_.at (i).toMap ();
			if (serverInfo ["Network"] ==  network &&
					serverInfo ["Server"] == server)
			{
				QStringList channels = serverInfo ["Channels"].toStringList ();
				for (int j = 0; j < channels.count (); ++j)
					if (channels [j].split (' ').at (0) == channel.toString ())
					{
						channels [j] = dec->GetChannel () + QString (" ") + dec->GetPassword ();
						break;
					}
				QMap<QString, QVariant> map = serverInfo;
				map ["Channels"] = channels;
				ServersInfo_ [i] = map;
			}
		}

		SetServers (ServersInfo_);
	}

	void IrcAccountConfigurationDialog::EditServer ()
	{
		std::auto_ptr<IrcAddServerDialog> dsa (new IrcAddServerDialog (0));

		QStringList channels;
		QModelIndex currentIndex = Ui_.ServerChannels_->currentIndex ();
		Q_FOREACH (const QVariant& serverInfo, ServersInfo_)
			if (serverInfo.toMap () ["Server"] == currentIndex.data () &&
					serverInfo.toMap () ["Network"] == currentIndex.parent ().data ())
			{
				dsa->SetServer (serverInfo.toMap () ["Server"].toString ());
				dsa->SetPassword (serverInfo.toMap () ["Password"].toString ());
				dsa->SetPort (serverInfo.toMap () ["Port"].toInt ());
				dsa->SetSSL (serverInfo.toMap () ["SSL"].toBool ());
				channels = serverInfo.toMap () ["Channels"].toStringList ();
			}

		if (dsa->exec () == QDialog::Rejected)
			return;

		QMap<QString, QVariant> server;
		server ["Network"] = Ui_.Networks_->text ();
		server ["Server"] = dsa->GetServer ();
		server ["Port"] = dsa->GetPort ();
		server ["Password"] = dsa->GetPassword ();
		server ["SSL"] = dsa->GetSSL ();
		server ["Channels"] = channels;

		for (int i = 0; i < ServersInfo_.count (); ++i)
			if (ServersInfo_.at (i).toMap () ["Server"] == currentIndex.data () &&
					ServersInfo_.at (i).toMap () ["Network"] == currentIndex.parent ().data ())
			{
				ServersInfo_ [i] = server;
				break;
			}

		SetServers (ServersInfo_);
	}

	QString IrcAccountConfigurationDialog::GetChannelPassword (const QString& server, 
			const QString& network, const QString& channel)
	{
		Q_FOREACH (const QVariant& serverInfo, ServersInfo_)
			if (serverInfo.toMap () ["Network"].toString () == network &&
					serverInfo.toMap () ["Server"].toString () == server)
			{
				QStringList channels = serverInfo.toMap () ["Channels"].toStringList ();
				Q_FOREACH (const QString& chnnl, channels)
					if (chnnl.split (' ').at (0) == channel)
						if (chnnl.split (' ').count () > 1)
							return chnnl.split (' ').at (1);
			}

		return QString ();
	}

	QMap<QString, QVariant> IrcAccountConfigurationDialog::GetNicknameData (const QString& network)
	{
		QMap<QString, QVariant> map;
		map ["Nicknames"] = Ui_.NickNames_->toPlainText ().split ('\n');
		map ["AutoGenerate"] = Ui_.GenerateNicknames_->isChecked ();
		map ["Network"] = network;

		return map;
	}

	bool IrcAccountConfigurationDialog::IsServerExists (const QMap< QString, QVariant >& server)
	{
		Q_FOREACH (const QVariant& serverInfo, ServersInfo_)
		{
			if (serverInfo.toMap () ["Network"] == server ["Network"] &&
					serverInfo.toMap () ["Server"] == server ["Server"] &&
					serverInfo.toMap () ["Port"] == server ["Port"] &&
					serverInfo.toMap () ["SSL"] == server ["SSL"])
				return true;
		}
		
		return false;
	}

	QStringList IrcAccountConfigurationDialog::RemoveDuplicatesChannels (const QString& network, 
			const QString& server, const QStringList& channels)
	{
		QStringList strList = channels;
		
		Q_FOREACH (const QVariant& serverInfo, ServersInfo_)
		{
			if (serverInfo.toMap () ["Network"].toString () == network &&
					serverInfo.toMap () ["Server"].toString () == server)
			{
				Q_FOREACH (const QString& str, serverInfo.toMap () ["Channels"].toStringList ())
				{
					for (int i = 0; i < strList.count (); ++i)
					{
						if (str.split (' ', QString::SkipEmptyParts).at (0) == 
								strList.at (i).split (' ', QString::SkipEmptyParts).at (0))
						{
							strList.removeAt (i);
							break;
						}
					}
				}
			}
		}
		
		return strList;
	}

	void IrcAccountConfigurationDialog::handleChangeServer (int index)
	{
		bool found = false;
		QString lastNetwork = Ui_.Servers_->itemText (LastIndex_);

		for (int i = 0; i < Nicknames_.count (); ++i)
			if (Nicknames_.at (i).toMap () ["Network"].toString () == lastNetwork)
			{
				Nicknames_ [i] = GetNicknameData (lastNetwork);
				found = true;
				break;
			}

		if (!found)
			Nicknames_ << GetNicknameData (lastNetwork);

		Ui_.NickNames_->setPlainText (QString ());
		Q_FOREACH (const QVariant& item, Nicknames_)
		{
			if (item.toMap () ["Network"].toString () == Ui_.Servers_->currentText ())
			{
				Ui_.NickNames_->setPlainText (item.toMap () ["Nicknames"].toStringList ().join ("\n"));
				Ui_.GenerateNicknames_->setChecked (item.toMap () ["AutoGenerate"].toBool ());
			}
			LastIndex_ = Ui_.Servers_->currentIndex ();
		}
	}

	void IrcAccountConfigurationDialog::handleAddServer (bool checked)
	{
		std::auto_ptr<IrcAddServerDialog> dsa (new IrcAddServerDialog (0));
		
		if (dsa->exec () == QDialog::Rejected)
			return;
		
		QMap<QString, QVariant> server;
		server ["Network"] = Ui_.Networks_->text ();
		server ["Server"] = dsa->GetServer ();
		server ["Port"] = dsa->GetPort ();
		server ["Password"] = dsa->GetPassword ();
		server ["SSL"] = dsa->GetSSL ();
		
		if (IsServerExists (server))
			return;
		
		ServersInfo_ << server;
		
		QString network = Ui_.Networks_->text ();
		QList<QStandardItem*> networkList = ServerAndChannels_->findItems (network);
		
		if (networkList.isEmpty ())
		{
			QStandardItem *item = new QStandardItem (network);
			item->setData ("network", ServerAndChannelsRole);
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
		
		QModelIndex currentIndex = Ui_.ServerChannels_->currentIndex (); 
		QModelIndex serverIndex = currentIndex;
		
		if (currentIndex.data (ServerAndChannelsRole).toString () == "channel")
			serverIndex = currentIndex.parent ();
		
		QVariant server = serverIndex.data ();
		QVariant network = serverIndex.parent ().data ();
		
		if (dac->exec () == QDialog::Rejected)
			return;

		QStringList list = RemoveDuplicatesChannels (network.toString (),
				server.toString (), dac->GetChannels ());
		
		if (list.isEmpty ())
			return;
		
		for (int i = 0; i < ServersInfo_.count (); i++)
		{
			QMap<QString, QVariant> srv = ServersInfo_.at (i).toMap ();
			if (srv ["Server"] == server && 
					srv ["Network"] == network)
				srv ["Channels"] = srv ["Channels"].toStringList () + 
						RemoveDuplicatesChannels (network.toString (),
								server.toString (), dac->GetChannelsPair ());
			ServersInfo_ [i] = srv;
		}

		QList<QStandardItem*> channelsList;
		Q_FOREACH (const QString& channel, list)
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
	
	void IrcAccountConfigurationDialog::handleNetworkTextChange (const QString& text)
	{
		AddServer_->setEnabled (!text.isEmpty ());
	}
	
	void IrcAccountConfigurationDialog::handleEditElement (bool checked)
	{
		if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "channel")
			EditChannel ();
		else if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "server")
			EditServer ();
	}

	void IrcAccountConfigurationDialog::handleDeleteElement (bool checked)
	{
		if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "network")
			DeleteElement ("Network", Ui_.ServerChannels_->currentIndex (),
					tr ("All servers and channels of this network will be delete too.\nAre you sure?"));
		else if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "server")
			DeleteElement ("Server", Ui_.ServerChannels_->currentIndex (),
					tr ("All channels of this server will be delete too.\nAre you sure?"));
		else if (Ui_.ServerChannels_->currentIndex ().data (ServerAndChannelsRole).toString () == "channel")
			DeleteChannel (Ui_.ServerChannels_->currentIndex (),
					tr ("Are you really want to delete this channel?"));
	}

	void IrcAccountConfigurationDialog::handleTabChange (int index)
	{
		if (!index)
		{
			QStringList list;
			int rowCount = ServerAndChannels_->rowCount ();
			if (!rowCount)
				list << Ui_.Servers_->itemText (0);
			else
				for (int i = 0; i < rowCount; ++i)
					list << ServerAndChannels_->item (i)->text ();

			SetNetworks (list);
		}
		else
		{
			bool found = false;
			for (int i = 0; i < Nicknames_.count (); ++i)
				if (Nicknames_.at (i).toMap () ["Network"].toString () == Ui_.Servers_->currentText ())
				{
					Nicknames_ [i] = GetNicknameData (Ui_.Servers_->currentText ());
					found = true;
				}
				
			if (!found)
				Nicknames_ << GetNicknameData (Ui_.Servers_->currentText ());
		}
	}
	
	void IrcAccountConfigurationDialog::handleDblClick (const QModelIndex& index)
	{
		if (index.isValid ())
			handleEditElement (false);
	}
};
};
};
