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

#include "syncer.h"
#include <QIcon>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Syncer
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("syncer"));

		Core::Instance ().SetProxy (proxy);

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"syncersettings.xml");

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().SecondInit ();
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

	QStringList Plugin::Provides () const
	{
		return QStringList ("syncplugin");
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_syncer, LeechCraft::Syncer::Plugin);
