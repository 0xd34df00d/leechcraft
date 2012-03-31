/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "xproxy.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include "proxyfactory.h"
#include "proxiesconfigwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace XProxy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;

		qRegisterMetaTypeStreamOperators<Proxy> ("LeechCraft::XProxy::Proxy");
		qRegisterMetaTypeStreamOperators<ReqTarget> ("LeechCraft::XProxy::ReqTarget");
		qRegisterMetaTypeStreamOperators<QList<LeechCraft::XProxy::Entry_t>> ("QList<LeechCraft::XProxy::Entry_t>");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "xproxysettings.xml");

		CfgWidget_ = new ProxiesConfigWidget ();
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
		QNetworkProxyFactory::setApplicationProxyFactory (app ? new ProxyFactory (CfgWidget_) : 0);

		const bool nam = XmlSettingsManager::Instance ().property ("EnableForNAM").toBool ();
		CoreProxy_->GetNetworkAccessManager ()->setProxyFactory (nam ? new ProxyFactory (CfgWidget_) : 0);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_xproxy, LeechCraft::XProxy::Plugin);
