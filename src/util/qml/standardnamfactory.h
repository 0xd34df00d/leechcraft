/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QtGlobal>
#include <QQmlNetworkAccessManagerFactory>
#include "qmlconfig.h"

class QQmlEngine;

namespace LC::Util
{
	/** @brief A standard QML QNetworkAccessManager factory.
	 *
	 * StandardNAMFactory allows easily creating QNetworkAccessManager
	 * instances in QML contexts.
	 *
	 * The created managers are all using the same cache, located at the
	 * cache path passed and limited by the maximum size passed to the
	 * constructor.
	 *
	 * Several different factories may be created sharing the same cache
	 * location. In this case, the minimum value of the cache size would
	 * be used as the maximum.
	 *
	 * @ingroup QmlUtil
	 */
	class UTIL_QML_API StandardNAMFactory : public QQmlNetworkAccessManagerFactory
	{
		const QString Subpath_;
	public:
		/** @brief The type of the function used to query the cache size
		 * by the factory.
		 */
		using CacheSizeGetter_f = std::function<int ()>;
	private:
		CacheSizeGetter_f CacheSizeGetter_;
	public:
		/** @brief Constructs a new StandardNAMFactory.
		 *
		 * The cache uses a subdirectory \em subpath in the \em network
		 * directory of the user cache location.
		 *
		 * @param[in] subpath The subpath in cache user location.
		 * @param[in] getter The function that would be queried during
		 * periodical cache garbage collection to fetch the current
		 * maximum cache size.
		 * @param[in] engine The QML engine where this factory should be
		 * installed, if not null.
		 */
		StandardNAMFactory (QString subpath,
				CacheSizeGetter_f getter,
				QQmlEngine *engine = nullptr);

		/** @brief Creates the network access manager with the given
		 * \em parent.
		 *
		 * The ownership of the returned QNetworkAccessManager is passed
		 * to the caller.
		 *
		 * @param[in] parent The parent of the QNetworkAccessManager to
		 * be created.
		 * @return A new QNetworkAccessManager.
		 */
		QNetworkAccessManager* create (QObject *parent) override;
	};
}
