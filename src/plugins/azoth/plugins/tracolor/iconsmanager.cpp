/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "iconsmanager.h"
#include <cmath>
#include <QPainter>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <interfaces/an/constants.h>
#include "entryeventsmanager.h"
#include "eventssettingsmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	IconsManager::IconsManager (EntryEventsManager *evMgr,
			EventsSettingsManager *settingsMgr, QObject *parent)
	: QObject { parent }
	, EvMgr_ { evMgr }
	, SettingsMgr_ { settingsMgr }
	{
		connect (EvMgr_,
				&EntryEventsManager::entryEventRateChanged,
				this,
				[this] (const GlobalStrongestId& entryId)
				{
					RegenCache (entryId);
					emit iconUpdated (entryId);
				});
		connect (SettingsMgr_,
				&EventsSettingsManager::eventsSettingsChanged,
				this,
				&IconsManager::RegenCaches);

		XmlSettingsManager::Instance ().RegisterObject ("HidingThreshold", this, [this] { RegenCaches (); });
	}

	QList<QIcon> IconsManager::GetIcons (const GlobalStrongestId& entryId)
	{
		if (!IconsCache_.contains (entryId))
			RegenCache (entryId);

		return Util::Map (IconsCache_.value (entryId), &IconsCacheEntry::Icon_);
	}

	void IconsManager::RegenCache (const GlobalStrongestId& entryId)
	{
		const auto tolerance = 10;

		const auto hideThreshold = XmlSettingsManager::Instance ()
				.property ("HidingThreshold").toInt ();

		QHash<QString, IconsCacheEntry> icons;
		for (const auto& pair : Util::Stlize (SettingsMgr_->GetEnabledEvents ()))
		{
			const auto rate = EvMgr_->GetEntryEventRate (entryId, pair.first.toUtf8 ());
			if (rate < std::numeric_limits<double>::epsilon () * tolerance)
				continue;

			QColor color { pair.second.Color_ };
			color.setAlphaF (rate);
			if (color.alpha () < hideThreshold)
				continue;

			QPixmap px { 22, 22 };
			px.fill (Qt::transparent);
			{
				QPainter p { &px };
				p.setBrush ({ color });
				p.setPen (Qt::NoPen);
				p.drawEllipse ({ px.width () / 2, px.height () / 2 },
						px.width () / 4, px.height () / 4);
			}

			icons.insert (pair.first, { px, color, rate });
		}

		IconsCache_ [entryId] = icons;
	}

	void IconsManager::RegenCaches ()
	{
		for (const auto& pair : Util::Stlize (std::exchange (IconsCache_, {})))
		{
			RegenCache (pair.first);

			const auto& oldEntries = pair.second;
			const auto& newEntries = IconsCache_.value (pair.first);

			const auto& stlized = Util::Stlize (newEntries);
			if (oldEntries.size () != newEntries.size () ||
					std::any_of (stlized.begin (), stlized.end (),
							[&oldEntries] (const auto& newEntryPair)
							{
								const auto& oldEntry = oldEntries.value (newEntryPair.first);
								const auto& newEntry = newEntryPair.second;
								return oldEntry.Color_ != newEntry.Color_ ||
										std::abs (oldEntry.Rate_ - newEntry.Rate_) >= 0.05;
							}))
				emit iconUpdated (pair.first);
		}
	}
}
}
}
