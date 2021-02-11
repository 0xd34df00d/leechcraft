/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <list>
#include <QHash>
#include <util/sll/typegetter.h>

namespace libtorrent
{
	class alert;
	class session;
}

namespace LC::BitTorrent
{
	class AlertDispatcher
	{
		libtorrent::session& Session_;

		using Handler_f = std::function<bool (const libtorrent::alert&)>;
		QHash<int, std::list<Handler_f>> Handlers_;

		class RegGuard
		{
			std::list<Handler_f>& HandlerList_;
			std::list<Handler_f>::iterator Pos_;

			RegGuard (std::list<Handler_f>& list, std::list<Handler_f>::iterator pos)
			: HandlerList_ { list }
			, Pos_ { pos }
			{
			}
		public:
			RegGuard (RegGuard&&) = delete;
			RegGuard (const RegGuard&) = delete;
			RegGuard& operator= (RegGuard&&) = delete;
			RegGuard& operator= (const RegGuard&) = delete;

			~RegGuard ()
			{
				HandlerList_.erase (Pos_);
			}
		};
	public:
		explicit AlertDispatcher (libtorrent::session&);

		template<typename F>
		int RegisterHandler (F&& f)
		{
			using AlertType_t = std::decay_t<Util::ArgType_t<F, 0>>;
			auto& list = Handlers_ [AlertType_t::alert_type];
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
			return AlertType_t::alert_type;
		}

		template<typename F>
		RegGuard RegisterTemporaryHandler (F&& f)
		{
			auto alertType = RegisterPermanentHandler (std::forward<F> (f));
			auto& list = Handlers_ [alertType];
			return RegGuard { list, list.begin () };
		}

		void Swallow (int alertType, bool logging);

		void PollAlerts ();
	private:
		void DispatchAlert (const libtorrent::alert&) const;
	};
}
