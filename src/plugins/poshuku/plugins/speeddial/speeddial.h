/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QAbstractItemModel>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihavesettings.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC::Poshuku
{
class IProxyObject;

namespace SpeedDial
{
	class ImageCache;
	class CustomSitesManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.SpeedDial")

		IProxyObject *PoshukuProxy_;
		ImageCache *Cache_;
		CustomSitesManager *CustomSites_;

		Util::XmlSettingsDialog_ptr XSD_;
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
	public slots:
		void initPlugin (QObject*);

		void hookBrowserWidgetInitialized (LC::IHookProxy_ptr, QObject*);
	};
}
}
