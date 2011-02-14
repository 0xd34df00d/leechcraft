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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "ui_ircaccountconfigurationdialog.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccountConfigurationDialog : public QDialog
	{
		Q_OBJECT
		
		enum ModelRole 
		{ 
			ServerAndChannelsRole = Qt::UserRole + 1
		};
		
		enum ServerRole 
		{ 
			DefaultServerRole = Qt::UserRole + 1
		};
		
		Ui::IrcAccountConfigurationDialog Ui_;
		QList<NickNameData> Nicknames_;
		int LastIndex_;
		QMenu *AddMenu_;
		QAction *AddChannel_;
		QAction *AddServer_;
		QStandardItemModel *ServerAndChannels_;
		QList<ServerInfoData> ServersInfo_;
	public:
		IrcAccountConfigurationDialog (QWidget* = 0);
		QString GetDefaultNickname () const;
		QList<NickNameData>  GetNicks ();
		void SetNicks (const QList<NickNameData> &);
		void SetDefaultServers (const QList<ServerInfoData>&);
		QList<ServerInfoData> GetServersInfo () const;
		void SetServersInfo (const QList<ServerInfoData>&);
	private:
		QStringList GetServers () const;
		void SetServers (const QStringList&);
		void DeleteNetwork (const QModelIndex&, const QString&);
		void DeleteServer (const QModelIndex&, const QString&);
		void DeleteChannel (const QModelIndex&, const QString&);
		void EditServer ();
		void EditChannel ();
		QString GetChannelPassword (const QString&, const QString&, const QString&);
		NickNameData GetNicknameData (const QString&, const QString&);
		bool IsServerExists (const ServerInfoData&);
		QStringList RemoveDuplicatesChannels (const QString&, const QString&, const QStringList&);
	public slots:
		void handleChangeServer (int);
		void handleAddServer (bool);
		void handleAddChannel (bool);
		void handleChannelsEnable (QModelIndex);
		void handleNetworkTextChange (const QString&);
		void handleEditElement (bool);
		void handleDeleteElement (bool);
		void handleTabChange (int);
		void handleDblClick (const QModelIndex&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONDIALOG_H
