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
		QHash<QString, boost::function<void (const QStringList&)> > Command2Signal_;
	public:
		IrcParser (IrcServer*);
		void AuthCommand (const ServerOptions&);
		void UserCommand (const ServerOptions&);
		void NickCommand (const ServerOptions&);
		void JoinChannel (const ChannelOptions&);
		void PrivMessageCommand (const QString&, const ServerOptions&, const ChannelOptions&);
		void HandleServerReply (const QString&);
	private:
		void Init ();
		void ParseMessage (const QString&);
		void ParsePrefix (const QString&);
	private slots:
		void pongCommand (const QStringList&);
	signals:
		void gotAuthSuccess (const QStringList&);
		void gotCLEntries (const QStringList&);
		void gotTopic (const QStringList&);
		void gotPing (const QStringList&);
		void gotMessage (const QStringList&s);
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
