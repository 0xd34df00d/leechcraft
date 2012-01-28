/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <interfaces/iproxyobject.h>
#include "msnprotocol.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	Core::Core ()
	: Protocol_ (new MSNProtocol)
	, ProxyObject_ (0)
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SecondInit ()
	{
		Protocol_->Init ();
	}

	void Core::SetPluginProxy (QObject *obj)
	{
		ProxyObject_ = qobject_cast<IProxyObject*> (obj);
	}

	IProxyObject* Core::GetPluginProxy() const
	{
		return ProxyObject_;
	}

	MSNProtocol* Core::GetProtocol () const
	{
		return Protocol_;
	}

	void Core::SendEntity (const Entity& e)
	{
		emit gotEntity (e);
	}
}
}
}
