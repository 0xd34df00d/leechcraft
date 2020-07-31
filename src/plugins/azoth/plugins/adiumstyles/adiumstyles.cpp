/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "adiumstyles.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include <util/util.h>
#include "adiumstylesource.h"

namespace LC
{
namespace Azoth
{
namespace AdiumStyles
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_adiumstyles");
		CoreProxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.AdiumStyles";
	}

	QString Plugin::GetName () const
	{
		return "Azoth AdiumStyles";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for Adium chat styles.");
	}

	QIcon Plugin::GetIcon () const
	{
		return CoreProxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin";
		return result;
	}

	QList<QObject*> Plugin::GetResourceSources () const
	{
		return ResourceSources_;
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Proxy_ = qobject_cast<IProxyObject*> (proxy);
		ResourceSources_ << new AdiumStyleSource (Proxy_);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_adiumstyles, LC::Azoth::AdiumStyles::Plugin);
