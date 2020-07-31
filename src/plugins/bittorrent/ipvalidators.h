/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QValidator>

namespace LC
{
namespace BitTorrent
{
	class ValidateIPv4 : public QValidator
	{
	public:
		ValidateIPv4 (QObject* = 0);

		State validate (QString&, int&) const;
	};

	class ValidateIPv6 : public QValidator
	{
	public:
		ValidateIPv6 (QObject* = 0);

		State validate (QString&, int&) const;
	};
}
}
