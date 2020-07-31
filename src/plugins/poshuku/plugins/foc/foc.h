/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/iwebplugin.h>
#include <interfaces/poshuku/iwebpluginprovider.h>

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	class FlashOnClickPlugin;
	class FlashOnClickWhitelist;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IWebPluginProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings LC::Poshuku::IWebPluginProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.FOC")

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr XSD_;

		std::shared_ptr<FlashOnClickPlugin> FlashOnClickPlugin_;
		FlashOnClickWhitelist *FlashOnClickWhitelist_ = nullptr;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QList<IWebPlugin*> GetWebPlugins () override;
	};
}
}
}
