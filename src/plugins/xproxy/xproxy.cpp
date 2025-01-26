/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xproxy.h"
#include <QIcon>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include "proxyfactory.h"
#include "proxiesconfigwidget.h"
#include "xmlsettingsmanager.h"
#include "proxiesstorage.h"
#include "scriptsmanager.h"
#include "urllistscript.h"

namespace LC
{
namespace XProxy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("xproxy");

		CoreProxy_ = proxy;

#if QT_VERSION_MAJOR == 5
		qRegisterMetaTypeStreamOperators<Proxy> ("LC::XProxy::Proxy");
		qRegisterMetaTypeStreamOperators<ReqTarget> ("LC::XProxy::ReqTarget");
		qRegisterMetaTypeStreamOperators<QList<LC::XProxy::Entry_t>> ("QList<LC::XProxy::Entry_t>");
		qRegisterMetaTypeStreamOperators<QList<LC::XProxy::ScriptEntry_t>> ("QList<LC::XProxy::ScriptEntry_t>");
#endif

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "xproxysettings.xml");

		const auto scriptsMgr = new ScriptsManager { proxy };
		Storage_ = new ProxiesStorage { scriptsMgr} ;

		CfgWidget_ = new ProxiesConfigWidget { Storage_, scriptsMgr };
		XSD_->SetCustomWidget ("Proxies", CfgWidget_);

		XmlSettingsManager::Instance ().RegisterObject ("EnableForNAM", this, "handleReenable");
		XmlSettingsManager::Instance ().RegisterObject ("EnableForApp", this, "handleReenable");
		handleReenable ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.XProxy";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "XProxy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Advanced proxy servers manager for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::handleReenable ()
	{
		const bool app = XmlSettingsManager::Instance ().property ("EnableForApp").toBool ();
		QNetworkProxyFactory::setApplicationProxyFactory (app ? new ProxyFactory (Storage_) : 0);

		const bool nam = XmlSettingsManager::Instance ().property ("EnableForNAM").toBool ();
		CoreProxy_->GetNetworkAccessManager ()->setProxyFactory (nam ? new ProxyFactory (Storage_) : 0);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_xproxy, LC::XProxy::Plugin);
