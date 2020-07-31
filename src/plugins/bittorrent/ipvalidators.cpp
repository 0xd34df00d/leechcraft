/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ipvalidators.h"
#include <QStringList>

namespace LC
{
namespace BitTorrent
{
	ValidateIPv4::ValidateIPv4 (QObject *parent)
	: QValidator (parent)
	{
	}

	QValidator::State ValidateIPv4::validate (QString& input, int&) const
	{
		const auto& octets = input.splitRef ('.', Qt::SkipEmptyParts);
		if (octets.size () != 4)
			return Invalid;
		for (const auto& octet : octets)
		{
			if (octet.isEmpty ())
				return Intermediate;
			int val = octet.toInt ();
			if (val < 0 || val > 255)
				return Invalid;
		}
		return Acceptable;
	}

	ValidateIPv6::ValidateIPv6 (QObject *parent)
	: QValidator (parent)
	{
	}

	QValidator::State ValidateIPv6::validate (QString& input, int&) const
	{
		if (input.count ("::") > 1)
			return Intermediate;

		const auto& octets = input.split (':', Qt::SkipEmptyParts);
		if (octets.size () != 8)
			return Invalid;
		for (const auto& octet : octets)
		{
			if (octet.isEmpty ())
				return Intermediate;
			int val = octet.toInt (nullptr, 16);
			if (val < 0 || val > 65535)
				return Invalid;
		}
		return Acceptable;
	}
}
}
