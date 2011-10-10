/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "knowhow.h"
#include <QIcon>
#include <QTimer>
#include <QDomDocument>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "tipdialog.h"

namespace LeechCraft
{
namespace KnowHow
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("knowhow");

		Proxy_ = proxy;

		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"knowhowsettings.xml");
	}

	void Plugin::SecondInit ()
	{
		if (XmlSettingsManager::Instance ()
				.property ("ShowTips").toBool ())
			QTimer::singleShot (10000,
					this,
					SLOT (showTip ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.KnowHow";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "KnowHow";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple plugin providing various tips of the day regarding LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::showTip ()
	{
		new TipDialog (Proxy_);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_knowhow, LeechCraft::KnowHow::Plugin);
