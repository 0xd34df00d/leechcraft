#ifndef PLUGINS_AGGREGATOR_ATOM03PARSER_H
#define PLUGINS_AGGREGATOR_ATOM03PARSER_H
#include <QPair>
#include <QDateTime>
#include "atomparser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class Atom03Parser : public AtomParser
			{
			    Atom03Parser ();
			public:
			    static Atom03Parser& Instance ();
			    virtual bool CouldParse (const QDomDocument&) const;
			private:
				channels_container_t Parse (const QDomDocument&) const;
			    Item* ParseItem (const QDomElement&) const;
			};
		};
	};
};

#endif

