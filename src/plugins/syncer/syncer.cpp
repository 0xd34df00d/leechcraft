/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncer.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/isyncable.h>
#include "xmlsettingsmanager.h"
#include "syncablemanager.h"

namespace LC
{
namespace Syncer
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"syncersettings.xml");

		SyncableMgr_ = new SyncableManager ();
	}

	void Plugin::SecondInit ()
	{
		for (auto plugin : Proxy_->GetPluginsManager ()->GetAllCastableTo<ISyncable*> ())
			SyncableMgr_->AddPlugin (plugin);
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Syncer";
	}

	QString Plugin::GetName () const
	{
		return "Syncer";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Synchronization plugin for LeechCraft");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_syncer, LC::Syncer::Plugin);
