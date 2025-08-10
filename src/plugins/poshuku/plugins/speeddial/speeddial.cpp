/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "speeddial.h"
#include <QIcon>
#include <QUrlQuery>
#include <interfaces/poshuku/iproxyobject.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include <interfaces/poshuku/iwebview.h>
#include "imagecache.h"
#include "customsitesmanager.h"
#include "xmlsettingsmanager.h"
#include "requesthandler.h"

namespace LC::Poshuku::SpeedDial
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		qRegisterMetaType<AddrList_t> ("LC::Poshuku::SpeedDial::AddrList_t");
		qRegisterMetaType<AddrList_t> ("LeechCraft::Poshuku::SpeedDial::AddrList_t");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "poshukuspeeddialsettings.xml");

		CustomSites_ = new CustomSitesManager;
		XSD_->SetDataSource ("SitesView", CustomSites_->GetModel ());
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
		delete Cache_;
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.SpeedDial";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku SpeedDial";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds a special speed dial page.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Poshuku.Plugins/1.0" };
	}

	void Plugin::initPlugin (QObject *object)
	{
		PoshukuProxy_ = qobject_cast<IProxyObject*> (object);

		Cache_ = new ImageCache { *PoshukuProxy_ };
	}

	void Plugin::hookBrowserWidgetInitialized (LC::IHookProxy_ptr,
			QObject *browserWidget)
	{
		qobject_cast<IBrowserWidget*> (browserWidget)->GetWebView ()->Load (SpeedDialUrl);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	auto Plugin::HandleRequest (const Request& request) -> HandleResult
	{
		if (request.Url_.host () != SpeedDialHost)
			return HandleResult { Error::Unsupported };

		return SpeedDial::HandleRequest (request.Url_.path (), QUrlQuery { request.Url_ },
				{ .CustomSites_ = *CustomSites_, .PoshukuProxy_ = *PoshukuProxy_, .ImageCache_ = *Cache_ });
	}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_speeddial, LC::Poshuku::SpeedDial::Plugin)
