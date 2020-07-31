/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QList>
#include <QDateTime>

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	class EntryEventsManager : public QObject
	{
		Q_OBJECT

		struct EntryEventInfo
		{
			QDateTime DT_;
			double Rate_;
		};
		QHash<QByteArray, QHash<QByteArray, EntryEventInfo>> EntryEvents_;
	public:
		EntryEventsManager (QObject* = nullptr);

		void AddEntry (QObject*);
		void RemoveEntry (QObject*);

		void HandleEvent (const QByteArray& entryId, const QByteArray& eventId);

		double GetEntryEventRate (const QByteArray& entryId, const QByteArray& eventId) const;
	private slots:
		void decayRates ();
	signals:
		void entryEventRateChanged (const QByteArray&);
	};
}
}
}
