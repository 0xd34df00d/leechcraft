/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <list>
#include <memory>
#include <QHash>
#include <util/sll/scopeguards.h>
#include <util/sll/typegetter.h>

namespace libtorrent
{
	class alert;
	class session;
}

namespace LC::BitTorrent
{
	using AlertHandler_f = std::function<bool (const libtorrent::alert&)>;

	class AlertDispatcher
	{
		libtorrent::session& Session_;
		QHash<int, std::list<AlertHandler_f>> Handlers_;
	public:
		explicit AlertDispatcher (libtorrent::session&);

		template<typename F>
		int RegisterHandler (F&& f)
		{
			using AlertType_t = std::decay_t<Util::ArgType_t<F, 0>>;
			constexpr int type = AlertType_t::alert_type;
			auto& list = Handlers_ [type];
			list.push_front ([f] (const libtorrent::alert& alert)
					{
						if constexpr (std::is_same_v<Util::RetType_t<F>, bool>)
							return f (static_cast<const AlertType_t&> (alert));
						else
						{
							f (static_cast<const AlertType_t&> (alert));
							return true;
						}
					});
			return type;
		}

		template<typename F>
		auto RegisterTemporaryHandler (F&& f)
		{
			auto alertType = RegisterHandler (std::forward<F> (f));
			auto& list = Handlers_ [alertType];
			return Util::MakeScopeGuard ([&list, it = list.begin ()] { list.erase (it); });
		}

		void Swallow (int alertType, bool logging);

		void PollAlerts ();
	private:
		void DispatchAlert (const libtorrent::alert&) const;
	};
}
