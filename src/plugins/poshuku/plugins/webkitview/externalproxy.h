/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class IEntityManager;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class ExternalProxy : public QObject
	{
		Q_OBJECT

		IEntityManager *const IEM_;
	public:
		ExternalProxy (IEntityManager*, QObject* = nullptr);
	public slots:
		void AddSearchProvider (const QString&);
	};
}
}
}
