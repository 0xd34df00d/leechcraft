/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <string_view>

namespace LC::Util
{
	namespace
	{
		template<typename K, typename V>
		struct KVPair
		{
			const K Key_;
			const V Val_;

			consteval KVPair (K name, V val)
			: Key_ { name }
			, Val_ { val }
			{
			}
		};

		template<size_t N, typename V>
		KVPair (const char (&) [N], V) -> KVPair<std::string_view, V>;
	}

	template<typename K>
	constexpr uint64_t DefaultHashImpl (K);

	template<>
	constexpr uint64_t DefaultHashImpl (std::string_view name)
	{
		uint64_t res = 0;
		for (auto ch : name)
			res = (res << 8) + ch;
		return res;
	}

	template<std::convertible_to<uint64_t> K>
	constexpr uint64_t DefaultHashImpl (K val)
	{
		return static_cast<uint64_t> (val);
	}

	constexpr uint64_t DefaultHash (auto val)
	{
		return DefaultHashImpl (val);
	}

	template<typename K, typename V, auto Hasher = DefaultHash<K>>
	consteval auto MakeHash (auto&&... commands)
	{
		const std::initializer_list<KVPair<K, V>> commandsList { commands... };
		for (auto i = commandsList.begin (); i != std::prev (commandsList.end ()); ++i)
			for (auto j = std::next (i); j != commandsList.end (); ++j)
				if (Hasher (i->Key_) == Hasher (j->Key_))
					throw "duplicate hashes";

		return [=] (K key, V defValue = V {})
		{
			const auto keyHash = Hasher (key);
			V result = defValue;
			K foundKey {};
			(void) ((Hasher (commands.Key_) == keyHash && (result = commands.Val_, foundKey = commands.Key_, true)) || ...);
			if (foundKey != key)
				return defValue;
			return result;
		};
	}

	template<typename V>
	consteval auto MakeStringHash (auto&&... commands)
	{
		return MakeHash<std::string_view, V> (std::forward<decltype (commands)> (commands)...);
	}
}
