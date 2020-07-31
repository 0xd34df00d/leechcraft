/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "foc.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "flashonclickplugin.h"
#include "flashonclickwhitelist.h"

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("poshuku_foc");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "poshukufocsettings.xml");

		FlashOnClickWhitelist_ = new FlashOnClickWhitelist;
		XSD_->SetCustomWidget ("FlashOnClickWhitelist", FlashOnClickWhitelist_);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.FOC";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku FOC";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Shows the Flash-on-Click placeholder instead of flash player clips.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QList<IWebPlugin*> Plugin::GetWebPlugins ()
	{
		if (!FlashOnClickPlugin_)
			FlashOnClickPlugin_ = std::make_shared<FlashOnClickPlugin> (Proxy_, FlashOnClickWhitelist_);

		return { FlashOnClickPlugin_.get () };
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_foc, LC::Poshuku::FOC::Plugin);
