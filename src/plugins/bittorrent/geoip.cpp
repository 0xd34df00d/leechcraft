/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "geoip.h"
#include <QStringList>
#include <QFile>
#include <QtDebug>

#ifdef ENABLE_GEOIP
#include <maxminddb.h>
#endif

namespace LC::BitTorrent
{
#ifdef ENABLE_GEOIP
	namespace
	{
		std::optional<QString> FindDB ()
		{
			const QStringList geoipCands
			{
				"/usr/share/GeoIP",
				"/usr/local/share/GeoIP",
				"/var/lib/GeoIP"
			};

			for (const auto& cand : geoipCands)
			{
				const auto& name = cand + "/GeoLite2-Country.mmdb";
				if (QFile::exists (name))
					return { name };
			}

			return {};
		}
	}

	GeoIP::GeoIP ()
	{
		const auto maybeImpl = FindDB ()
				.and_then ([] (const QString& path) -> std::optional<ImplPtr_t>
					{
						qDebug () << Q_FUNC_INFO << "loading GeoIP from" << path;

						MMDB_s mmdb;
						if (int status = MMDB_open (path.toStdString ().c_str (), MMDB_MODE_MMAP, &mmdb);
							status != MMDB_SUCCESS)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to load MaxMind DB from:"
									<< status;
							return {};
						}

						auto ptr = ImplPtr_t { new MMDB_s, &MMDB_close };
						*ptr = mmdb;
						return { ptr };
					});
		Impl_ = maybeImpl.value_or (ImplPtr_t {});
	}

	std::optional<QString> GeoIP::GetCountry (const libtorrent::address& addr) const
	{
		if (!Impl_)
			return {};

		int gai_error;
		int mmdb_error;
		auto entry = MMDB_lookup_string (Impl_.get (), addr.to_string ().c_str (), &gai_error, &mmdb_error);
		if (gai_error != 0 || mmdb_error != MMDB_SUCCESS)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to query MMDB for"
					<< addr.to_string ().c_str ();
			return {};
		}

		if (!entry.found_entry)
			return {};

		MMDB_entry_data_s entryData;
		if (int result = MMDB_get_value (&entry.entry, &entryData, "country", "iso_code", NULL);
			result != MMDB_SUCCESS || !entryData.has_data || !entryData.utf8_string)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to query MMDB entry for the country iso code:"
					<< result
					<< entryData.has_data;
			return {};
		}

		return QString::fromLatin1 (entryData.utf8_string, 2).toLower ();
	}
#else
	GeoIP::GeoIP ()
	{
	}

	std::optional<QString> GeoIP::GetCountry (const libtorrent::address&) const
	{
		return {};
	}
#endif

	GeoIP& GeoIP::Instance ()
	{
		static GeoIP geoIP;
		return geoIP;
	}
}
