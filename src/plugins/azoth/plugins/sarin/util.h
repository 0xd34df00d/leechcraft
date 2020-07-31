/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <array>
#include <optional>
#include <QByteArray>

typedef struct Tox Tox;

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	std::optional<qint32> GetFriendId (const Tox *tox, const QByteArray& pubkey);
	QByteArray GetFriendId (const Tox *tox, int32_t friendId);

	template<size_t Size>
	QByteArray ToxId2HR (const uint8_t *address)
	{
		QByteArray result;
		auto toHexChar = [] (uint8_t num) -> char
		{
			return num >= 10 ? (num - 10 + 'A') : (num + '0');
		};

		for (size_t i = 0; i < Size; ++i)
		{
			const auto num = address [i];
			result += toHexChar ((num & 0xf0) >> 4);
			result += toHexChar (num & 0xf);
		}

		return result;
	}

	template<size_t Size>
	QByteArray ToxId2HR (const std::array<uint8_t, Size>& address)
	{
		return ToxId2HR<Size> (address.data ());
	}
}
}
}
