/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "ipvalidators.h"
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			ValidateIPv4::ValidateIPv4 (QObject *parent)
			: QValidator (parent)
			{
			}

			QValidator::State ValidateIPv4::validate (QString& input, int&) const
			{
				QStringList octets = input.split ('.', QString::SkipEmptyParts);
				if (octets.size () != 4)
					return Invalid;
				Q_FOREACH (QString octet, octets)
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

				QStringList octets = input.split (':', QString::SkipEmptyParts);
				if (octets.size () != 8)
					return Invalid;
				Q_FOREACH (QString octet, octets)
				{
					if (octet.isEmpty ())
						return Intermediate;
					int val = octet.toInt (0, 16);
					if (val < 0 || val > 65535)
						return Invalid;
				}
				return Acceptable;
			}
		};
	};
};

