#ifndef OPMLPARSER_H
#define OPMLPARSER_H
#include <vector>
#include <QHash>
#include <QString>
#include <QDomDocument>
#include "opmlitem.h"

class QDomElement;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class OPMLParser
			{
			public:
				typedef std::vector<OPMLItem> items_container_t;
				typedef QHash<QString, QString> OPMLinfo_t;
			private:
				mutable items_container_t Items_;
				mutable bool CacheValid_;
				QDomDocument Document_;
			public:
				OPMLParser (const QDomDocument&);

				void Reset (const QDomDocument&);
				bool IsValid () const;
				OPMLinfo_t GetInfo () const;
				items_container_t Parse () const;
			private:
				void ParseOutline (const QDomElement&,
						QStringList = QStringList ()) const;
			};
		};
	};
};

#endif

