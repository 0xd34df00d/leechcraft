/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "lmp.h"
#include <QIcon>
#include <QFileInfo>
#include <QUrl>
#include <phonon/mediaobject.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/entitytesthandleresult.h>
#include "playertab.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpsettings.xml");

		PlayerTC_ =
		{
			GetUniqueID () + "_player",
			"LMP",
			GetInfo (),
			GetIcon (),
			40,
			TFSingle | TFOpenableByRequest
		};

		Core::Instance ().SetProxy (proxy);

		Core::Instance ().PostInit ();

		PlayerTab_ = new PlayerTab (PlayerTC_, this);
		connect (PlayerTab_,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (PlayerTab_,
				SIGNAL (changeTabName (QWidget*, QString)),
				this,
				SIGNAL (changeTabName (QWidget*, QString)));
		connect (PlayerTab_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "LMP";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LeechCraft Music Player.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/lmp/resources/images/lmp.svg");
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t tcs;
		tcs << PlayerTC_;
		return tcs;
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == PlayerTC_.TabClass_)
		{
			emit addNewTab ("LMP", PlayerTab_);
			emit raiseTab (PlayerTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		QString path = e.Entity_.toString ();
		const QUrl& url = e.Entity_.toUrl ();
		if (path.isEmpty () &&
					url.isValid () &&
					url.scheme () == "file")
			path = url.toLocalFile ();

		if (!path.isEmpty ())
		{
			const auto& goodExt = XmlSettingsManager::Instance ()
					.property ("TestExtensions").toString ()
					.split (' ', QString::SkipEmptyParts);
			const QFileInfo fi = QFileInfo (path);
			if (fi.exists () && goodExt.contains (fi.suffix ()))
				return EntityTestHandleResult (EntityTestHandleResult::PHigh);
			else
				return EntityTestHandleResult ();
		}

		return EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		QString path = e.Entity_.toString ();
		const QUrl& url = e.Entity_.toUrl ();
		if (path.isEmpty () &&
					url.isValid () &&
					url.scheme () == "file")
			path = url.toLocalFile ();

		if (e.Parameters_ & Internal)
		{
			auto obj = Phonon::createPlayer (Phonon::NotificationCategory, path);
			obj->play ();
			connect (obj,
					SIGNAL (finished ()),
					obj,
					SLOT (deleteLater ()));
		}
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp, LeechCraft::LMP::Plugin);
