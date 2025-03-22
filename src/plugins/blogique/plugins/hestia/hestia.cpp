/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hestia.h"
#include <QIcon>
#include "localbloggingplatform.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"blogiquehestiasettings.xml");

		Platform_ = std::make_shared<LocalBloggingPlatform> (this);
	}

	void Plugin::SecondInit ()
	{
		Platform_->Prepare ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blogique.Hestia";
	}

	void Plugin::Release ()
	{
		Platform_->Release ();
	}

	QString Plugin::GetName () const
	{
		return "Blogique Hestia";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Local blogging platform plugin for Blogique.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Blogique.Plugins.IBlogPlatformPlugin" };
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetBloggingPlatforms () const
	{
		return { Platform_.get () };
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Platform_->SetPluginProxy (proxy);
	}

}
}
}

LC_EXPORT_PLUGIN (leechcraft_blogique_hestia, LC::Blogique::Hestia::Plugin);
