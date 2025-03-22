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
#include "standardstylesource.h"

namespace LC::Azoth::StandardStyles
{
	void Plugin::Init (ICoreProxy_ptr)
	{
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
		return { Source_ };
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Source_ = new StandardStyleSource { qobject_cast<IProxyObject*> (proxy) };
	}

	void Plugin::hookThemeReloaded (const LC::IHookProxy_ptr&, QObject*, QWebEngineView *view, QObject*)
	{
		Source_->PrepareColors (view);
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_standardstyles, LC::Azoth::StandardStyles::Plugin);
