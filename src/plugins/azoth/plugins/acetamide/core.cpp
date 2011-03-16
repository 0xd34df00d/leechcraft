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

#include "core.h"
#include <ctime>
#include <interfaces/iaccount.h>
#include <interfaces/iproxyobject.h>
#include "ircaccount.h"
#include "ircprotocol.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	Core::Core ()
	: PluginProxy_ (0)
	, DefaultAccount_ (0)
	, DefaultUserName_ (QString ())
	{
		qRegisterMetaTypeStreamOperators<ServerOptions> ("ServerOptions");
		qRegisterMetaTypeStreamOperators<ChannelOptions> ("ChannelOptions");
		
		IrcProtocol_.reset (new IrcProtocol (this));
		SocketManager_.reset (new SocketManager (this));
		ServerManager_.reset (new IrcServerManager (this));
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SecondInit ()
	{
		IrcProtocol_->SetProxyObject (PluginProxy_);
		IrcProtocol_->Prepare ();
	}

	void Core::Release ()
	{
		SocketManager_.reset ();
		ServerManager_.reset ();
		IrcProtocol_.reset ();
	}

	QList<QObject*> Core::GetProtocols () const
	{
		QList<QObject*> result;
		result << qobject_cast<QObject*> (IrcProtocol_.get ());
		return result;
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SetDefaultIrcAcoount (IrcAccount *account)
	{
		DefaultAccount_ = account;
	}

	IrcAccount* Core::GetDefaultIrcAccount ()
	{
		if (!DefaultAccount_)
			CreateDefaultAccount ();
		return DefaultAccount_;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}

	QString Core::GetDefaultUserName () const
	{
		return DefaultUserName_;
	}

	boost::shared_ptr<SocketManager> Core::GetSocketManager () const
	{
		return SocketManager_;
	}

	boost::shared_ptr<IrcServerManager> Core::GetServerManager () const
	{
		return ServerManager_;
	}

	void Core::CreateDefaultAccount ()
	{
		QString defaultAccountName ("DefaultIrcAccount");
		QString defaultNick = QString ("leechcraft") + 
				QString::number (10 + qrand () % 89);
		
		DefaultAccount_ = new IrcAccount (defaultAccountName, IrcProtocol_.get ());
		
		QList<ServerOptions> defaultAccServers = DefaultAccount_->
				ReadServersSettings (defaultAccountName + "_Servers");
		if (!defaultAccServers.isEmpty ())
		{
			if (defaultAccServers.first ().ServerNicknames_.isEmpty ())
				defaultAccServers [0].ServerNicknames_ << defaultNick;
		}
		else
		{
			ServerOptions server;
			server.NetworkName_ = tr ("Default");
			server.ServerName_ = tr ("Default");
			server.ServerNicknames_ << defaultNick;
			DefaultUserName_ = "Mr.Leechcraft";
			server.ServerRealName_ = DefaultUserName_;
			server.ServerPassword_ = QString ();
			server.ServerPort_ = 0;
			server.SSL_ = false;
			server.ServerEncoding_ = "UTF-8";
			defaultAccServers.append (server);
		}
		
		DefaultAccount_->SaveServersSettings (defaultAccServers, 
				defaultAccountName + "_Servers");
	}

	void Core::handleItemsAdded (const QList<QObject*>& items)
	{
	}
}
}
}