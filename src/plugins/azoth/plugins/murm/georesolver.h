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
#include <QHash>
#include <QSet>

template<typename>
class QFuture;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkConnection;
	enum class GeoIdType;

	class GeoResolver : public QObject
	{
		Q_OBJECT

		VkConnection * const Conn_;

		QHash<int, QString> Countries_;
		QHash<int, QString> Cities_;

		QSet<int> PendingCountries_;
		QSet<int> PendingCities_;
	public:
		GeoResolver (VkConnection*, QObject* = 0);

		void CacheCountries (QList<int>);
		void AddCountriesToCache (const QHash<int, QString>&);
		QFuture<QString> RequestCountry (int);
		QString GetCountry (int) const;

		void CacheCities (QList<int>);
		void AddCitiesToCache (const QHash<int, QString>&);
		QFuture<QString> RequestCity (int);
		QString GetCity (int) const;
	private:
		void Cache (QList<int>, QHash<int, QString>&, QSet<int>&, GeoIdType);
		QFuture<QString> Get (int, QHash<int, QString>&, GeoIdType);
	};
}
}
}
