/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/blogique/ibloggingplatformplugin.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	class LocalBloggingPlatform;

	class Plugin : public QObject
				, public IInfo
				, public IHaveSettings
				, public IPlugin2
				, public IBloggingPlatformPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings
				LC::Blogique::IBloggingPlatformPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Blogique.Hestia")

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		std::shared_ptr<LocalBloggingPlatform> Platform_;
	public:
		void Init (ICoreProxy_ptr proxy) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QObject* GetQObject () override;
		QList<QObject*> GetBloggingPlatforms () const override;

	public slots:
		void initPlugin (QObject *proxy);
	};
}
}
}
