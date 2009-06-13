#ifndef XBELPARSER_H
#define XBELPARSER_H
#include <QStringList>

class QByteArray;
class QDomElement;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class XbelParser
			{
			public:
				XbelParser (const QByteArray&);
			private:
				void ParseFolder (const QDomElement&, QStringList = QStringList ());
			};
		};
	};
};

#endif

