/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QNetworkAccessManager>
#include "interfaces/core/ihookproxy.h"

namespace LC::Util
{
	class CustomCookieJar;
}

namespace LC
{
	class CookieSaver;

	class NetworkAccessManager : public QNetworkAccessManager
	{
		Q_OBJECT

		std::unique_ptr<Util::CustomCookieJar> CookieJar_;
		std::unique_ptr<CookieSaver> CookieSaver_;
	public:
		explicit NetworkAccessManager (QObject* = nullptr);
		~NetworkAccessManager () override;
	protected:
		QNetworkReply* createRequest (Operation, const QNetworkRequest&, QIODevice*) override;
	signals:
		void requestCreated (QNetworkAccessManager::Operation,
				const QNetworkRequest&, QNetworkReply*);

		void hookNAMCreateRequest (LC::IHookProxy_ptr proxy,
					QNetworkAccessManager *manager,
					QNetworkAccessManager::Operation *op,
					QIODevice **dev);
	};
};
