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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcServer;
	
	class IrcParser : public QObject
	{
		Q_OBJECT
		IrcServer *IrcServer_;
		QString Prefix_;
		QString Nick_;
		QString User_;
		QString Host_;
		QString ServerName;
		QString Command_;
		QStringList Parameters_;
		QHash<QString, 
				boost::function<void (const QString&, const QList<std::string>&, const QString&)> > Command2Signal_;
	public:
		IrcParser (IrcServer*);
		void AuthCommand (const ServerOptions&);
		void UserCommand (const ServerOptions&);
		void NickCommand (const ServerOptions&);
		void JoinChannel (const ChannelOptions&);
		void PublicMessageCommand (const QString&, const ChannelOptions&);
		void PrivateMessageCommand (const QString&, const QString&);
		void HandleServerReply (const QString&);
		void LeaveChannelCommand (const QString&);
		void QuitConnectionCommand (const QString&);
		QString GetNickName () const;
	private:
		void Init ();
		void ParseMessage (const QString&);
		void ParsePrefix (const QString&);
	private slots:
		void pongCommand (const QString&, const QList<std::string>&, const QString&);
	signals:
		void gotAuthSuccess (const QString&, const QList<std::string>&, const QString&);
		void gotCLEntries (const QString&, const QList<std::string>&, const QString&);
		void gotTopic (const QString&, const QList<std::string>&, const QString&);
		void gotPing (const QString&, const QList<std::string>&, const QString&);
		void gotMessage (const QString&, const QList<std::string>&, const QString&);
		void gotNewParticipant (const QString&, const QList<std::string>&, const QString&);
		void gotUserLeave (const QString&, const QList<std::string>&, const QString&);
		void gotUserQuit (const QString&, const QList<std::string>&, const QString&);
		void gotServerSupport (const QString&, const QList<std::string>&, const QString&);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
