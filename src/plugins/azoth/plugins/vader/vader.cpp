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

#include "vader.h"
#include <QIcon>
#include <QAction>
#include <QUrl>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "mrimprotocol.h"
#include "mrimbuddy.h"
#include "vaderutil.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
        Util::InstallTranslator ("azoth_vader");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothvadersettings.xml");

		Core::Instance ().SetCoreProxy (proxy);
		Core::Instance ().GetProtocol ()->setParent (this);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().GetProtocol ()->Init ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Vader";
	}

	QString Plugin::GetName () const
	{
		return "Azoth Vader";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for the Mail.ru Agent protocol.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/vader/resources/images/vader.svg");
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
		return classes;
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QList<QObject*> Plugin::GetProtocols () const
	{
		return QList<QObject*> () << Core::Instance ().GetProtocol ();
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Core::Instance ().SetProxy (proxy);
	}

	void Plugin::hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
			QObject *action,
			QObject *entry)
	{
	}

	void Plugin::hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
			QObject *entry)
	{
		if (!qobject_cast<MRIMBuddy*> (entry))
			return;

		if (!EntryServices_.contains (entry))
		{
			auto list = VaderUtil::GetBuddyServices (this,
					SLOT (entryServiceRequested ()));
			Q_FOREACH (QAction *act, list)
				act->setProperty ("Azoth/Vader/Entry", QVariant::fromValue<QObject*> (entry));
			EntryServices_ [entry] = list;
		}

		QList<QVariant> list = proxy->GetReturnValue ().toList ();
		Q_FOREACH (QAction *act, EntryServices_ [entry])
			list += QVariant::fromValue<QObject*> (act);
		proxy->SetReturnValue (list);
	}

	void Plugin::entryServiceRequested ()
	{
		const QString& url = sender ()->property ("URL").toString ();
		QObject *buddyObj = sender ()->property ("Azoth/Vader/Entry").value<QObject*> ();
		MRIMBuddy *buddy = qobject_cast<MRIMBuddy*> (buddyObj);
		const QString& subst = VaderUtil::SubstituteNameDomain (url,
				buddy->GetHumanReadableID ());
		const Entity& e = Util::MakeEntity (QUrl (subst),
				QString (),
				static_cast<LeechCraft::TaskParameters> (OnlyHandle | FromUserInitiated));
		emit gotEntity (e);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_vader, LeechCraft::Azoth::Vader::Plugin);
