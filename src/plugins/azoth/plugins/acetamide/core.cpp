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
	{
		IrcProtocol_.reset (new IrcProtocol (this));
		qRegisterMetaTypeStreamOperators<NickNameData> ("NickNameData");
		qRegisterMetaTypeStreamOperators<ServerInfoData> ("ServerInfoData");
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

	void Core::CreateDefaultAccount ()
	{
		DefaultAccount_ = new IrcAccount (tr ("DefaultIrcAccount"), IrcProtocol_.get ());
		QList<NickNameData> defaultAcc = DefaultAccount_->ReadNicknameSettings ("DefaultIrcAccount_Nicknames");
		if (!defaultAcc.isEmpty ())
		{
			if (defaultAcc.first ().Nicks_.isEmpty ())
				defaultAcc [0].Nicks_ << QString ("leechraft") + QString::number (10 + qrand () % 89);
		}
		else
		{
			NickNameData acc;
			acc.Server_ = "default";
			acc.ServerName_ = tr ("Default");
			acc.Nicks_ << QString ("leechraft") + QString::number (10 + qrand () % 99);
			acc.AutoGenerate_ = true;
			defaultAcc << acc;
		}
		DefaultAccount_->SaveNicknameSettings (defaultAcc, "DefaultIrcAccount_Nicknames");
	}

	void Core::handleItemsAdded (const QList<QObject*>& items)
	{
	}
}
}
}