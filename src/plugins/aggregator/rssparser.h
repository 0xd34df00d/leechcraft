#ifndef PLUGINS_AGGREGATOR_RSSPARSER_H
#define PLUGINS_AGGREGATOR_RSSPARSER_H
#include <QMap>
#include <QString>
#include "parser.h"
#include "channel.h"

class QDomDocument;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class RSSParser : public Parser
			{
			protected:
				QMap<QString, int> TimezoneOffsets_;

				RSSParser ();
			public:
				virtual ~RSSParser ();
				channels_container_t Parse (const channels_container_t&,
						channels_container_t&,
						const QDomDocument&) const;
			protected:
				virtual channels_container_t Parse (const QDomDocument&) const = 0;
				QDateTime RFC822TimeToQDateTime (const QString&) const;
				QList<Enclosure> GetEnclosures (const QDomElement&) const;
			};
		};
	};
};

#endif

