/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_TYPESFACTORY_H
#define PLUGINS_QROSP_WRAPPERS_TYPESFACTORY_H
#include <QObject>

namespace LC
{
namespace Qrosp
{
	class TypesFactory : public QObject
	{
		Q_OBJECT
	public:
		TypesFactory (QObject* = 0);
	public slots:
		QObject* Create (const QString&);
	};
}
}

#endif
