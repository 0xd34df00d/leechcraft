/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "../azothcommon.h"
#include <util/azoth/emitters/core/entriesmanager.h>

#include "interfaces/azoth/iclentry.h"

namespace LC::Azoth
{
	class IEntriesManager
	{
	protected:
		Emitters::EntriesManager Emitter_;

		virtual ~IEntriesManager () = default;
	public:
		const Emitters::EntriesManager& GetEmitter () const { return Emitter_; }

		virtual std::optional<quint64> GetSessionStableId (const GlobalStrongestId& id) const = 0;

		quint64 GetSessionStableId (const ICLEntry& entry) const
		{
			const auto id = entry.GetGlobalStrongestId ();
			if (const auto val = GetSessionStableId (id))
				return *val;

			throw std::runtime_error { "Unknown entry " + id.ToString ().toStdString () };
		}
	};
}
