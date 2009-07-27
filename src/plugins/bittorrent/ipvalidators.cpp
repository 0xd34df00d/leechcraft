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

