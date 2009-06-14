#ifndef PLUGINS_AGGREGATOR_RSS10PARSER_H
#define PLUGINS_AGGREGATOR_RSS10PARSER_H
#include "rssparser.h"
#include "channel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class RSS10Parser : public RSSParser
			{
				RSS10Parser ();
			public:
				virtual ~RSS10Parser ();
			    static RSS10Parser& Instance ();
			    virtual bool CouldParse (const QDomDocument&) const;
			private:
				channels_container_t Parse (const QDomDocument&) const;
			};
		};
	};
};

#endif

