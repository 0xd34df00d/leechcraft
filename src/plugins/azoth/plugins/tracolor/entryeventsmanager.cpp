/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entryeventsmanager.h"
#include <cmath>
#include <QTimer>
#include <util/sll/qtutil.h>
#include <interfaces/azoth/iclentry.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	EntryEventsManager::EntryEventsManager (QObject *parent)
	: QObject { parent }
	{
		auto timer = new QTimer { this };
		timer->start (10000);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (decayRates ()));

		XmlSettingsManager::Instance ().RegisterObject ("FadeoutTime", this, "decayRates");
	}

	void EntryEventsManager::AddEntry (QObject*)
	{
	}

	void EntryEventsManager::RemoveEntry (QObject *entryObj)
	{
		EntryEvents_.remove (qobject_cast<ICLEntry*> (entryObj)->GetEntryID ().toUtf8 ());
	}

	void EntryEventsManager::HandleEvent (const QByteArray& entryId, const QByteArray& eventId)
	{
		EntryEvents_ [entryId] [eventId].DT_ = QDateTime::currentDateTime ();
		emit entryEventRateChanged (entryId);
	}

	double EntryEventsManager::GetEntryEventRate (const QByteArray& entryId, const QByteArray& eventId) const
	{
		const auto& date = EntryEvents_.value (entryId).value (eventId).DT_;
		if (!date.isValid ())
			return 0;

		const auto fadeTime = XmlSettingsManager::Instance ().property ("FadeoutTime").toInt ();
		const auto minOp = XmlSettingsManager::Instance ().property ("HidingThreshold").toInt ();
		const auto diff = date.secsTo (QDateTime::currentDateTime ()) / 60.0;

		const auto minRate = minOp / 255.0;
		/* Hyperbolic case.
		const auto k = (1 / minRate - 1) / fadeTime;
		return 1 / (k * diff + 1);
		*/
		const auto k = (1 - minRate) / fadeTime;
		return std::max (1 - k * diff, 0.0);
	}

	void EntryEventsManager::decayRates ()
	{
		for (const auto& pair : Util::Stlize (EntryEvents_))
			emit entryEventRateChanged (pair.first);
	}
}
}
}
