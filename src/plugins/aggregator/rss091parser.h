#ifndef RSS091PARSER_H
#define RSS091PARSER_H
#include "rssparser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class RSS091Parser : public RSSParser
			{
				RSS091Parser ();
			public:
				virtual ~RSS091Parser ();
			    static RSS091Parser& Instance ();
			    virtual bool CouldParse (const QDomDocument&) const;
			protected:
				virtual channels_container_t Parse (const QDomDocument&) const;
				Item* ParseItem (const QDomElement&) const;
			};
		};
	};
};

#endif

