/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QWebKitPlatformPlugin>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>

namespace LC
{
namespace WKPlugins
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public QWebKitPlatformPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings QWebKitPlatformPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.WKPlugins")

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr XSD_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog() const override;

		bool supportsExtension (Extension) const override;
		QObject* createExtension (Extension) const override;
	};
}
}
