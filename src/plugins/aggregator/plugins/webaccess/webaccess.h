/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <QDir>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/aggregator/item.h>
#include <interfaces/aggregator/iaggregatorplugin.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihavesettings.h>

namespace LC
{
namespace Util
{
	class AddressesModelManager;
}

namespace Aggregator
{
namespace WebAccess
{
	class ServerManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IAggregatorPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveSettings
				LC::Aggregator::IAggregatorPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Aggregator.WebAccess")

		ICoreProxy_ptr Proxy_;
		std::shared_ptr<ServerManager> SM_;

		Util::XmlSettingsDialog_ptr XSD_;

		Util::AddressesModelManager *AddrMgr_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		void InitPlugin (IProxyObject*) override;
	};
}
}
}
