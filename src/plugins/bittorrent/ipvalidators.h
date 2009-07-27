#ifndef PLUGINS_TORRENT_IPVALIDATORS_H
#define PLUGINS_TORRENT_IPVALIDATORS_H
#include <QValidator>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class ValidateIPv4 : public QValidator
			{
				Q_OBJECT
			public:
				ValidateIPv4 (QObject* = 0);

				State validate (QString&, int&) const;
			};

			class ValidateIPv6 : public QValidator
			{
				Q_OBJECT
			public:
				ValidateIPv6 (QObject* = 0);

				State validate (QString&, int&) const;
			};
		};
	};
};

#endif

