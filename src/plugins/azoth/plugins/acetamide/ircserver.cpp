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

#include "ircserver.h"
#include "ircaccount.h"
#include "ircparser.h"


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServer::IrcServer (const ServerOptions& server, IrcAccount *account)
	: Account_ (account)
	, Server_ (server)
	, IsConnected (false)
	, Channels_ (QStringList ())
	, IrcParser_ (new IrcParser (this))
	{
		connect (this,
				SIGNAL (readyToReadAnswer (const QString&)),
				IrcParser_.get () ,
				SLOT (handleServerReply (const QString&)));
	}

	void IrcServer::JoinChannel (const ChannelOptions& channel)
	{
		ChannelsQueue_.append (channel);
		
		if (!IsConnected)
			ConnectToServer ();
		
		
	}
	
	void IrcServer::ConnectToServer ()
	{
		IrcParser_->AuthCommand (Server_);
	}

	IrcAccount* IrcServer::GetIrcAccount () const
	{
		return Account_;
	}

	boost::shared_ptr<IrcParser> IrcServer::GetParser () const
	{
		return IrcParser_;
	}

};
};
};
