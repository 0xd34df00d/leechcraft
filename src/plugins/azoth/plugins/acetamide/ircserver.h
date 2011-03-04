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

#include "boost/shared_ptr.hpp"
#include <QObject>
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	
	class IrcParser;
	class IrcAccount;
	
	class IrcServer : public QObject
	{
		Q_OBJECT
		
		boost::shared_ptr<IrcParser> IrcParser_;
		bool IsConnected;
		QStringList Channels_;
		QList<ChannelOptions> ChannelsQueue_;
		ServerOptions Server_;
		IrcAccount *Account_;
	public:
		IrcServer (const ServerOptions&, IrcAccount*);
		void JoinChannel (const ChannelOptions&);
		void ConnectToServer ();
		IrcAccount* GetIrcAccount () const;
		boost::shared_ptr<IrcParser> GetParser () const;
	signals:
		void readyToReadAnswer (const QString&);
	};
	
	typedef boost::shared_ptr<IrcServer> IrcServer_ptr;
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVER_H
