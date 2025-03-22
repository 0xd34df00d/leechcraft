/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webaccess.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/xsd/addressesmodelmanager.h>
#include "servermanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::AddressesModelManager::RegisterTypes ();

		AddrMgr_ = new Util::AddressesModelManager (&XmlSettingsManager::Instance (), 9001, this);

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "aggregatorwebaccesssettings.xml");

		XSD_->SetDataSource ("AddressesDataView", AddrMgr_->GetModel ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Aggregator.WebAccess";
	}

	void Plugin::Release ()
	{
		SM_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "Aggregator WebAccess";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides remote HTTP/Web access to Aggregator.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Aggregator.GeneralPlugin/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::InitPlugin (IProxyObject *proxy)
	{
		try
		{
			SM_ = std::make_shared<ServerManager> (proxy, Proxy_, AddrMgr_);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_aggregator_webaccess, LC::Aggregator::WebAccess::Plugin);
