#ifndef OPMLWRITER_H
#define OPMLWRITER_H
#include "feed.h"
#include <QString>

class QDomElement;
class QDomDocument;
class QDomNode;
class QStringList;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class OPMLWriter
			{
			public:
				OPMLWriter ();
				~OPMLWriter ();

				QString Write (const channels_shorts_t&,
						const QString&,
						const QString&,
						const QString&) const;
			private:
				void WriteHead (QDomElement&,
						QDomDocument&,
						const QString&,
						const QString&,
						const QString&) const;
				void WriteBody (QDomElement&,
						QDomDocument&,
						const channels_shorts_t&) const;
			};
		};
	};
};

#endif

