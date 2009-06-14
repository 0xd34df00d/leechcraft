#ifndef PLUGINS_AGGREGATOR_ATOMPARSER_H
#define PLUGINS_AGGREGATOR_ATOMPARSER_H
#include "parser.h"
#include "channel.h"

class QDomDocument;
class QString;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class AtomParser : public Parser
			{
			public:
				virtual channels_container_t Parse (const channels_container_t&,
						channels_container_t&,
						const QDomDocument&) const;
			protected:
				virtual channels_container_t Parse (const QDomDocument&) const = 0;
				virtual QString ParseEscapeAware (const QDomElement&) const;
				QList<Enclosure> GetEnclosures (const QDomElement&) const;
			};
		};
	};
};

#endif

