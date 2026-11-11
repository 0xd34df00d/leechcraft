/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QHashFunctions>
#include <util/threads/coro/taskfwd.h>
#include "../azothcommon.h"

namespace LC::Azoth
{
	class IAvatarsManager
	{
	protected:
		virtual ~IAvatarsManager () = default;
	public:
		struct OriginalSize
		{
			auto operator<=> (const OriginalSize&) const = default;

			friend size_t qHash (OriginalSize, size_t seed = 0) noexcept { return seed; }
		};

		struct Scaled
		{
			int Dim_;

			auto operator<=> (const Scaled&) const = default;

			friend size_t qHash (Scaled size, size_t seed = 0) noexcept { return qHash (size.Dim_, seed); }
		};

		static constexpr auto NotificationSize = Scaled { 128 };

		using AvatarSize = std::variant<OriginalSize, Scaled>;

		friend size_t qHash (const AvatarSize& size, size_t seed = 0) noexcept
		{
			return std::visit ([&] (const auto& alt) { return qHashMulti (seed, size.index (), alt); }, size);
		}

		/** @brief Returns the avatar data if it is present locally.
		 *
		 * - An empty `QByteArray` is returned if the entry is known to have
		 *   no avatar.
		 * - `std::nullopt` is returned if the avatar is not present locally.
		 *
		 * @note This function _does not_ do network requests.
		 */
		virtual Util::ContextTask<std::optional<QByteArray>> GetLocalAvatarData (GlobalStrongestId) = 0;

		/** @brief Returns the avatar image at the given size.
		 *
		 * @note Might fetch the avatar data from local cache or network.
		 */
		virtual Util::ContextTask<std::optional<QImage>> GetAvatarImage (GlobalStrongestId, AvatarSize) = 0;
	};
}
