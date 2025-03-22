/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nacheku.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include "xmlsettingsmanager.h"
#include "clipboardwatcher.h"
#include "directorywatcher.h"

namespace LC
{
namespace Nacheku
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "nachekusettings.xml");

		const auto iem = proxy->GetEntityManager ();

		new DirectoryWatcher { iem, this };
		new ClipboardWatcher { iem, this };
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Nacheku";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Nacheku";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Nacheku watches clipboard for links and a directory for files.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_nacheku, LC::Nacheku::Plugin);
