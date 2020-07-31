/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QDomDocument>
#include <QTimer>
#include <QDir>
#include <QXmppLogger.h>
#include <util/sys/paths.h>
#include <interfaces/azoth/iaccount.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	Core::Core ()
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SecondInit ()
	{
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}
}
}
}
