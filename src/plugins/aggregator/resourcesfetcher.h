/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include "common.h"

class QString;

class IEntityManager;

namespace LC::Aggregator
{
	class ResourcesFetcher : public QObject
	{
		IEntityManager * const EntityManager_;
	public:
		explicit ResourcesFetcher (IEntityManager*, QObject* = nullptr);

		void FetchPixmap (IDType_t, const QString&);
		void FetchFavicon (IDType_t, const QString&);
	private:
		void FetchExternalFile (const QString&, const std::function<void (QString)>&);
	};
}
