#include "pageformsdata.h"
#include <QtDebug>

QDebug& LeechCraft::Plugins::Poshuku::operator<< (QDebug& dbg, const ElementData& ed)
{
	dbg << "Element: {"
		<< ed.Name_
		<< ed.Type_
		<< ed.Value_
		<< "}";
	return dbg;
}


