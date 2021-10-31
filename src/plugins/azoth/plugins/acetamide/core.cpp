/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QStandardItemModel>
#include <QRegExp>
#include <interfaces/azoth/iproxyobject.h>
#include "ircprotocol.h"

namespace LC::Azoth::Acetamide
{
	Core::Core ()
	: IrcProtocol_ { std::make_shared<IrcProtocol> () }
	{
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
		return { IrcProtocol_.get () };
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	IProxyObject* Core::GetPluginProxy () const
	{
		return qobject_cast<IProxyObject*> (PluginProxy_);
	}
}
