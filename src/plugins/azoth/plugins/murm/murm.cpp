/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "murm.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iproxyobject.h>
#include "vkprotocol.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothmurmsettings.xml");

		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Murm";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Murm";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Native support for the VKontakte messaging protocol.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return { Proto_ };
	}

	void Plugin::initPlugin (QObject *azothProxy)
	{
		Proto_ = new VkProtocol (Proxy_, qobject_cast<IProxyObject*> (azothProxy), this);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_murm, LC::Azoth::Murm::Plugin);
