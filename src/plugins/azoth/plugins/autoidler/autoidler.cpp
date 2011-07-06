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

#include "autoidler.h"
#include <QIcon>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "3dparty/idle.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Autoidler
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_autoidler"));

		Proxy_ = proxy;

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog);
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothautoidlersettings.xml");
		
		Idle_.reset (new Idle);
		Idle_->start ();
		connect (Idle_.get (),
				SIGNAL (secondsIdle (int)),
				this,
				SLOT (handleIdle (int)));
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Autoidler";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Autoidler";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Automatically updates statuses depending on idle time.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}
	
	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
	
	void Plugin::handleIdle (int seconds)
	{
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_autoidler, LeechCraft::Azoth::Autoidler::Plugin);
