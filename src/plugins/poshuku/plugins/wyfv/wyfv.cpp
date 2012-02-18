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

#include "wyfv.h"
#include <typeinfo>
#include <QIcon>
#include <QtDebug>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "wyfvplugin.h"
#include "playerfactory.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace WYFV
{
	void WYFV::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku_wyfv"));

		Core::Instance ().SetProxy (proxy);

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"poshukuwyfvsettings.xml");
	}

	void WYFV::SecondInit ()
	{
	}

	void WYFV::Release ()
	{
	}

	QByteArray WYFV::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.WYFV";
	}

	QString WYFV::GetName () const
	{
		return "Poshuku WYFV";
	}

	QString WYFV::GetInfo () const
	{
		return tr ("Replaces Flash-based video player to play video without Flash installed.");
	}

	QIcon WYFV::GetIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/wyfv/resources/images/poshuku_wyfv.svg");
	}

	QStringList WYFV::Provides () const
	{
		return QStringList ();
	}

	QStringList WYFV::Needs () const
	{
		return QStringList ();
	}

	QStringList WYFV::Uses () const
	{
		return QStringList ();
	}

	void WYFV::SetProvider (QObject*, const QString&)
	{
	}

	std::shared_ptr<Util::XmlSettingsDialog> WYFV::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QSet<QByteArray> WYFV::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void WYFV::hookWebPluginFactoryReload (LeechCraft::IHookProxy_ptr,
			QList<IWebPlugin*>& plugins)
	{
		plugins << Core::Instance ().GetWYFVPlugin ();
	}

	bool WYFV::WouldOverrideFlash (const QUrl& url) const
	{
		return PlayerFactory::HasPlayerFor (url);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_wyfv, LeechCraft::Poshuku::WYFV::WYFV);
