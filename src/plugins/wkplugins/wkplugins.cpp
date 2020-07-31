/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wkplugins.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "staticplugin.h"
#include "notificationsext.h"
#include "spellcheckext.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace WKPlugins
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("wkplugins");

		Proxy_ = proxy;

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "wkpluginssettings.xml");

		StaticPlugin::SetImpl (this);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.WKPlugins";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "WKPlugins";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides support for spellchecking and HTML5 notifications to WebKit.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	bool Plugin::supportsExtension (Extension ext) const
	{
		switch (ext)
		{
		case Extension::Notifications:
		case Extension::SpellChecker:
			return true;
		default:
			return false;
		}
	}

	QObject* Plugin::createExtension (Extension ext) const
	{
		QObject *extObj = nullptr;
		switch (ext)
		{
		case Extension::Notifications:
			extObj = new NotificationsExt { Proxy_ };
			break;
		case Extension::SpellChecker:
			extObj = new SpellcheckerExt { Proxy_ };
			break;
		default:
			break;
		}
		return extObj;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_wkplugins, LC::WKPlugins::Plugin);

Q_IMPORT_PLUGIN (StaticPlugin)
