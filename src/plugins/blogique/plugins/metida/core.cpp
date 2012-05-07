/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include "interfaces/blogique/ipluginproxy.h"
#include "ljbloggingplatform.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	Core::Core ()
	: PluginProxy_ (0)
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SecondInit ()
	{
		if (LJPlatform_)
		{
			LJPlatform_->SetPluginProxy (PluginProxy_);
			LJPlatform_->Prepare ();
		}
	}

	void Core::CreateBloggingPlatfroms (QObject *parentPlatform)
	{
		LJPlatform_ = std::make_shared<LJBloggingPlatform> (parentPlatform);
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetCoreProxy ()
	{
		return Proxy_;
	}

	QObjectList Core::GetBloggingPlatforms () const
	{
		return LJPlatform_ ? QObjectList () << LJPlatform_.get () : QObjectList ();
	}

	void Core::SetPluginProxy (QObject *pluginProxy)
	{
		PluginProxy_ = pluginProxy;
	}

	IPluginProxy* Core::GetPluginProxy ()
	{
		return qobject_cast<IPluginProxy*> (PluginProxy_);
	}

}
}
}
