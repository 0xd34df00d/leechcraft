/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_XBELPARSER_H
#define PLUGINS_POSHUKU_XBELPARSER_H
#include <QStringList>

class QByteArray;
class QDomElement;

namespace LC
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
}
}

#endif
