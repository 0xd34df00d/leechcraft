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
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
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
		Util::InstallTranslator ("azoth_tracolor");

		const auto eventsSettingsManager = new EventsSettingsManager;
		EventsManager_ = new EntryEventsManager;
		IconsManager_ = new IconsManager { EventsManager_, eventsSettingsManager };
		connect (IconsManager_,
				SIGNAL (iconUpdated (QByteArray)),
				this,
				SLOT (handleIconsUpdated (QByteArray)));

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
				e.Additional_.contains ("org.LC.Plugins.Azoth.SourceID");
		return can ?
				EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
				EntityTestHandleResult {};
	}

	void Plugin::Handle (Entity e)
	{
		const auto& sourceId = e.Additional_ ["org.LC.Plugins.Azoth.SourceID"].toString ();
		const auto& eventId = e.Additional_ [AN::EF::EventType].toString ();

		EventsManager_->HandleEvent (sourceId.toUtf8 (), eventId.toUtf8 ());

		if (e.Additional_.contains ("org.LC.Plugins.Azoth.SubSourceID"))
		{
			const auto& subId = e.Additional_ ["org.LC.Plugins.Azoth.SubSourceID"].toString ();
			EventsManager_->HandleEvent (subId.toUtf8 (), eventId.toUtf8 ());
		}
	}

	void Plugin::initPlugin (QObject *proxyObj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	void Plugin::hookCollectContactIcons (IHookProxy_ptr,
			QObject *entryObj, QList<QIcon>& icons) const
	{
		if (!XmlSettingsManager::Instance ().property ("EnableTracolor").toBool ())
			return;

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto& sourceId = entry->GetEntryID ().toUtf8 ();

		const auto& newIcons = IconsManager_->GetIcons (sourceId);
		if (!newIcons.isEmpty ())
			icons = newIcons + icons;
	}

	void Plugin::handleIconsUpdated (const QByteArray& entryId)
	{
		if (!AzothProxy_)
			return;

		const auto entry = AzothProxy_->GetEntry (entryId);
		if (!entry)
			return;

		AzothProxy_->RedrawItem (entry);
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
