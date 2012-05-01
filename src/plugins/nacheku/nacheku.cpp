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

#include "nacheku.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "clipboardwatcher.h"
#include "directorywatcher.h"

namespace LeechCraft
{
namespace Nacheku
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XSD_.reset (new Util::XmlSettingsDialog ());
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "nachekusettings.xml");

		auto dirW = new DirectoryWatcher (this);
		connect (dirW,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		auto clipW = new ClipboardWatcher (this);
		connect (clipW,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
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

LC_EXPORT_PLUGIN (leechcraft_nacheku, LeechCraft::Nacheku::Plugin);

