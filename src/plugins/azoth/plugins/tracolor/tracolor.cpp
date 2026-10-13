/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tracolor.h"
#include <QIcon>
#include <QtDebug>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <util/sll/qobjectrefcast.h>
#include <util/azoth/anconstants.h>
#include "entryeventsmanager.h"
#include "iconsmanager.h"
#include "xmlsettingsmanager.h"
#include "eventssettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		const auto eventsSettingsManager = new EventsSettingsManager;
		EventsManager_ = new EntryEventsManager;
		IconsManager_ = new IconsManager { EventsManager_, eventsSettingsManager };
		connect (IconsManager_,
				&IconsManager::iconUpdated,
				this,
				[this] (const GlobalStrongestId& entryId)
				{
					if (const auto entry = AzothProxy_ ? AzothProxy_->GetEntry (entryId) : nullptr)
						AzothProxy_->RedrawItem (entry);
				});

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothtracolorsettings.xml");

		XSD_->SetDataSource ("EventsTypesDataView", eventsSettingsManager->GetModel ());

		XmlSettingsManager::Instance ().RegisterObject ("EnableTracolor",
				this, "handleEnableTracolorChanged");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Tracolor";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Tracolor";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Indicates contacts activity via color coding.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool can = e.Mime_.startsWith ("x-leechcraft/notification") &&
				e.Additional_ [AN::EF::EventCategory].toString () != AN::CatEventCancel &&
				e.Additional_.contains (Fields::Internal::SourceGlobalStrongestID);
		return can ?
				EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
				EntityTestHandleResult {};
	}

	void Plugin::Handle (Entity e)
	{
		const auto& sourceId = e.Additional_ [Fields::Internal::SourceGlobalStrongestID].value<GlobalStrongestId> ();
		const auto& eventId = e.Additional_ [AN::EF::EventType].toString ();

		EventsManager_->HandleEvent (sourceId, eventId.toUtf8 ());

		if (const auto roomIdVar = e.Additional_.value (Fields::Internal::RoomGlobalStrongestID);
			roomIdVar.isValid ())
		{
			const auto& roomId = roomIdVar.value<GlobalStrongestId> ();
			EventsManager_->HandleEvent (roomId, eventId.toUtf8 ());
		}
	}

	void Plugin::initPlugin (QObject *proxyObj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	void Plugin::hookCollectContactIcons (IHookProxy_ptr, QObject *entryObj, QList<QIcon>& icons) const
	{
		if (!XmlSettingsManager::Instance ().property ("EnableTracolor").toBool ())
			return;

		const auto& entry = qobject_ref_cast<ICLEntry> (entryObj);
		if (const auto& newIcons = IconsManager_->GetIcons (entry.GetGlobalStrongestID ());
			!newIcons.isEmpty ())
			icons = newIcons + icons;
	}

	void Plugin::handleEnableTracolorChanged ()
	{
		if (!AzothProxy_)
			return;

		for (const auto accObj : AzothProxy_->GetAllAccounts ())
			for (const auto entry : qobject_cast<IAccount*> (accObj)->GetCLEntries ())
				AzothProxy_->RedrawItem (entry);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_tracolor, LC::Azoth::Tracolor::Plugin);
