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

#include "xtazy.h"
#include <QIcon>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupporttune.h>
#include <interfaces/azoth/iproxyobject.h>
#include "tunesourcebase.h"
#include "xmlsettingsmanager.h"
#include "filesource.h"

#ifdef HAVE_DBUS
#include "mprissource.h"
#endif

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_xtazy"));

		AzothProxy_ = 0;
		Proxy_ = proxy;
		
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothxtazysettings.xml");
		
#ifdef HAVE_DBUS
		TuneSources_ << new MPRISSource (this);
#endif
		TuneSources_ << new FileSource (this);
		
		Q_FOREACH (TuneSourceBase *base, TuneSources_)
			connect (base,
					SIGNAL (tuneInfoChanged (const QMap<QString, QVariant>&)),
					this,
					SLOT (publish (const QMap<QString, QVariant>&)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Xtazy";
	}

	void Plugin::Release ()
	{
		qDeleteAll (TuneSources_);
	}

	QString Plugin::GetName () const
	{
		return "Azoth Xtazy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Publishes current tune.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/xtazy/resources/images/xtazy.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}
	
	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}
	
	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}
	
	void Plugin::publish (const QMap<QString, QVariant>& info)
	{
		const QByteArray& objName = sender ()->objectName ().toLatin1 ();
		
		if (!XmlSettingsManager::Instance ()
				.property ("Enable" + objName).toBool ())
			return;

		Q_FOREACH (QObject *accObj, AzothProxy_->GetAllAccounts ())
		{
			IAccount *acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
				continue;
			if (acc->GetState ().State_ == SOffline)
				continue;

			ISupportTune *tune = qobject_cast<ISupportTune*> (accObj);
			if (tune)
				tune->PublishTune (info);
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_xtazy, LeechCraft::Azoth::Xtazy::Plugin);

