/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "standardstyles.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include <util/util.h>
#include "standardstylesource.h"

namespace LC
{
namespace Azoth
{
namespace StandardStyles
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator (QStringLiteral ("azoth_standardstyles"));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.StandardStyles";
	}

	QString Plugin::GetName () const
	{
		return QStringLiteral ("Azoth StandardStyles");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for standard Azoth chat styles.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return
		{
			"org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin",
			"org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin"
		};
	}

	QList<QObject*> Plugin::GetResourceSources () const
	{
		return ResourceSources_;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Proxy_ = qobject_cast<IProxyObject*> (proxy);
		ResourceSources_ << new StandardStyleSource (Proxy_);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_standardstyles, LC::Azoth::StandardStyles::Plugin);
