/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "georesolver.h"
#include <QFuture>
#include <QFutureInterface>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include "vkconnection.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	GeoResolver::GeoResolver (VkConnection *conn, QObject *parent)
	: QObject (parent)
	, Conn_ (conn)
	{
	}

	void GeoResolver::CacheCountries (QList<int> ids)
	{
		Cache (ids, Countries_, PendingCountries_, GeoIdType::Country);
	}

	void GeoResolver::AddCountriesToCache (const QHash<int, QString>& countries)
	{
		Countries_.unite (countries);
	}

	QFuture<QString> GeoResolver::RequestCountry (int code)
	{
		return Get (code, Countries_, GeoIdType::Country);
	}

	QString GeoResolver::GetCountry (int code) const
	{
		return Countries_.value (code);
	}

	void GeoResolver::CacheCities (QList<int> ids)
	{
		Cache (ids, Cities_, PendingCities_, GeoIdType::Country);
	}

	void GeoResolver::AddCitiesToCache (const QHash<int, QString>& cities)
	{
		Cities_.unite (cities);
	}

	QFuture<QString> GeoResolver::RequestCity (int code)
	{
		return Get (code, Cities_, GeoIdType::City);
	}

	QString GeoResolver::GetCity (int code) const
	{
		return Cities_.value (code);
	}

	void GeoResolver::Cache (QList<int> ids, QHash<int, QString>& result, QSet<int>& pending, GeoIdType type)
	{
		auto newEnd = std::remove_if (ids.begin (), ids.end (),
				[&result, &pending] (int id)
				{
					return result.contains (id) || pending.contains (id);
				});
		ids.erase (newEnd, ids.end ());

		if (ids.isEmpty ())
			return;

		for (const auto id : ids)
			pending << id;

		Conn_->RequestGeoIds (ids,
				[&result, &pending] (const QHash<int, QString>& newItems)
				{
					result.unite (newItems);
					for (const auto& pair : Util::Stlize (newItems))
						pending.remove (pair.first);
				},
				type);
	}

	QFuture<QString> GeoResolver::Get (int code,
			QHash<int, QString>& hash, GeoIdType type)
	{
		if (hash.contains (code))
			return Util::MakeReadyFuture (hash [code]);

		QFutureInterface<QString> iface;
		iface.reportStarted ();
		Conn_->RequestGeoIds ({ code },
				[&hash, code, iface] (const QHash<int, QString>& result) mutable
				{
					hash.unite (result);

					const auto& value = result [code];
					iface.reportFinished (&value);
				},
				type);
		return iface.future ();
	}
}
}
}
