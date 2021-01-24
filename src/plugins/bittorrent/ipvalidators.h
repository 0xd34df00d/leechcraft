/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QValidator>

namespace LC::BitTorrent
{
	class ValidateIPv4 : public QValidator
	{
	public:
		using QValidator::QValidator;

		State validate (QString&, int&) const override;
	};

	class ValidateIPv6 : public QValidator
	{
	public:
		using QValidator::QValidator;

		State validate (QString&, int&) const override;
	};
}
