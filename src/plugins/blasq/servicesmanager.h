/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Blasq
{
	class IServicesPlugin;
	class IService;

	class ServicesManager : public QObject
	{
		Q_OBJECT

		QList<IService*> Services_;
	public:
		ServicesManager (QObject* = 0);

		void AddPlugin (IServicesPlugin*);

		const QList<IService*>& GetServices () const;
	signals:
		void serviceAdded (IService*);
	};
}
}
