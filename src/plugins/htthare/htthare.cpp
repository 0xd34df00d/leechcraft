/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "htthare.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/xsd/addressesmodelmanager.h>
#include "server.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace HttHare
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::AddressesModelManager::RegisterTypes ();

		AddrMgr_ = new Util::AddressesModelManager (&XmlSettingsManager::Instance (), 14801, this);
		connect (AddrMgr_,
				&Util::AddressesModelManager::addressesChanged,
				this,
				[this]
				{
					if (!S_)
						return;

					// The old server has to release its ports before the new one binds them.
					S_.reset ();
					S_ = std::make_shared<Server> (AddrMgr_->GetAddresses ());
					S_->Start ();
				});

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "httharesettings.xml");

		XSD_->SetDataSource ("AddressesDataView", AddrMgr_->GetModel ());

		XmlSettingsManager::Instance ().RegisterObject ("EnableServer",
				this, "handleEnableServerChanged");
		handleEnableServerChanged ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HttHare";
	}

	void Plugin::Release ()
	{
		S_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "HTThare";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Share your files over local network via HTTP.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::handleEnableServerChanged ()
	{
		const bool enable = XmlSettingsManager::Instance ().property ("EnableServer").toBool ();

		if (enable == static_cast<bool> (S_))
			return;

		if (S_)
			S_.reset ();
		else
		{
			S_.reset (new Server { AddrMgr_->GetAddresses () });
			S_->Start ();
		}
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_htthare, LC::HttHare::Plugin);
