/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "speeddial.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/poshuku/iproxyobject.h>
#include <interfaces/poshuku/ibrowserwidget.h>
#include "viewhandler.h"
#include "imagecache.h"
#include "customsitesmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Poshuku::SpeedDial
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		qRegisterMetaType<AddrList_t> ("LC::Poshuku::SpeedDial::AddrList_t");
		qRegisterMetaType<AddrList_t> ("LeechCraft::Poshuku::SpeedDial::AddrList_t");
		qRegisterMetaTypeStreamOperators<AddrList_t> ();

		Util::InstallTranslator ("poshuku_speeddial");

		Cache_ = new ImageCache { proxy };

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
	}

	void Plugin::hookBrowserWidgetInitialized (LC::IHookProxy_ptr,
			QObject *browserWidget)
	{
		new ViewHandler { qobject_cast<IBrowserWidget*> (browserWidget), Cache_, CustomSites_, PoshukuProxy_ };
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_speeddial, LC::Poshuku::SpeedDial::Plugin);
