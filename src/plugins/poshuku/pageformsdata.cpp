#include "pageformsdata.h"
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			QDebug& operator<< (QDebug& dbg, const ElementData& ed)
			{
				dbg << "Element: {"
					<< ed.Name_
					<< ed.Type_
					<< ed.Value_
					<< "}";
				return dbg;
			}
		};
	};
};

