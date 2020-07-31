/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rosenthal.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"
#include "knowndictsmanager.h"
#include "checker.h"

namespace LC
{
namespace Rosenthal
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("rosenthal");

		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"rosenthalsettings.xml");

		connect (SettingsDialog_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handlePushButtonClicked (QString)));

		KnownMgr_ = new KnownDictsManager;
		SettingsDialog_->SetDataSource ("Dictionaries", KnownMgr_->GetModel ());
		SettingsDialog_->SetDataSource ("PrimaryLanguage", KnownMgr_->GetEnabledModel ());
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Rosenthal";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Rosenthal";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Spellchecker service module for other plugins to use.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	ISpellChecker_ptr Plugin::CreateSpellchecker ()
	{
		return std::make_shared<Checker> (KnownMgr_);
	}

	void Plugin::handlePushButtonClicked (const QString& name)
	{
		if (name != "InstallDicts")
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown button"
					<< name;
			return;
		}

		auto e = Util::MakeEntity ("ListPackages",
				QString (),
				FromUserInitiated,
				"x-leechcraft/package-manager-action");
		e.Additional_ ["Tags"] = QStringList ("dicts");

		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_rosenthal, LC::Rosenthal::Plugin);
