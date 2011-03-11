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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVER_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcParser;
	class IrcAccount;
	class IrcServerManager;

	class IrcServer : public QObject
	{
		Q_OBJECT

		boost::shared_ptr<IrcParser> IrcParser_;
		QList<ChannelOptions> ChannelsQueue_;
		QList<ChannelOptions> ActiveChannels_;
		ServerOptions Server_;
		ConnectionState State_;
		IrcServerManager *ServerManager_;
		QString Nickname_;
	public:
		IrcServer (const ServerOptions&, IrcServerManager*);
		void JoinChannel (const ChannelOptions&);
		void ConnectToServer ();
		boost::shared_ptr<IrcParser> GetParser () const;
		QString GetHost () const;
		int GetPort () const;
		QString GetServerKey () const;
		ConnectionState GetState () const;
		QString GetNickName () const;
		QString GetEncoding () const;
		void AddChannel2Queue (const ChannelOptions&);
		void ChangeState (ConnectionState);
		void ReadAnswer (const QString&);
		void SendPublicMessage (const QString&, const ChannelOptions&);
		void LeaveChannel (const QString&);
	public slots:
		void authFinished (const QStringList&);
		void setTopic (const QStringList&);
		void setCLEntries (const QStringList&);
		void readMessage (const QStringList&);
		void setNewParticipant (const QStringList&);
		void setUserLeave (const QStringList&);
		void setUserQuit  (const QStringList&);
	};

	typedef boost::shared_ptr<IrcServer> IrcServer_ptr;
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVER_H
