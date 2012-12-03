/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-Rudoy
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

#include "dumbeep.h"
#include <interfaces/entitytesthandleresult.h>
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <QProcess>

#ifdef WITH_PHONON
#include <phonon/mediaobject.h>
#endif

#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Dumbeep
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "dumbeepsettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Dumbeep";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Dumbeep";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple audio notification backend for those fearing LMP.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	namespace
	{
		QString GetPath (const Entity& e)
		{
			QString path = e.Entity_.toString ();
			const QUrl& url = e.Entity_.toUrl ();
			if (path.isEmpty () &&
						url.isValid () &&
						url.scheme () == "file")
				path = url.toLocalFile ();
			return path;
		}
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (!XmlSettingsManager::Instance ().property ("PreferPhonon").toBool () &&
				XmlSettingsManager::Instance ().property ("PlayerCommand").toString ().isEmpty ())
			return EntityTestHandleResult ();

		if (!(e.Parameters_ & Internal))
			return EntityTestHandleResult ();

		const auto& path = GetPath (e);

		if (!path.isEmpty ())
		{
			const QStringList goodExt = { "mp3", "ogg", "wav", "flac" };
			const QFileInfo fi = QFileInfo (path);
			if (fi.exists () && goodExt.contains (fi.suffix ()))
				return EntityTestHandleResult (EntityTestHandleResult::PNormal);
			else
				return EntityTestHandleResult ();
		}

		return EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		const auto& path = GetPath (e);

		if (XmlSettingsManager::Instance ().property ("PreferPhonon").toBool ())
		{
#ifdef WITH_PHONON
			auto obj = Phonon::createPlayer (Phonon::NotificationCategory, path);
			obj->play ();
			connect (obj,
					SIGNAL (finished ()),
					obj,
					SLOT (deleteLater ()));
			return;
#endif
		}

		const auto& commandStr = XmlSettingsManager::Instance ()
				.property ("PlayerCommand").toString ();
		const auto& parts = commandStr.split (' ', QString::SkipEmptyParts);
		if (parts.isEmpty ())
			return;

		QProcess::startDetached (parts.at (0), parts.mid (1) << path);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_dumbeep, LeechCraft::Dumbeep::Plugin);
