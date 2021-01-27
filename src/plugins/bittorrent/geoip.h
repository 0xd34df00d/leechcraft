/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QString>
#include <libtorrent/address.hpp>

struct MMDB_s;

namespace LC::BitTorrent
{
	class GeoIP
	{
		using ImplPtr_t = std::shared_ptr<MMDB_s>;
		ImplPtr_t Impl_;

		GeoIP ();
	public:
		static GeoIP& Instance ();

		std::optional<QString> GetCountry (const libtorrent::address&) const;
	};
}
