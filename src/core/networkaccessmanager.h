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

class QTimer;

namespace LC
{
	namespace Util
	{
		class CustomCookieJar;
	}

	class NetworkAccessManager : public QNetworkAccessManager
	{
		Q_OBJECT

		std::unique_ptr<Util::CustomCookieJar> CookieJar_;
	public:
		explicit NetworkAccessManager (QObject* = nullptr);
		virtual ~NetworkAccessManager ();
	protected:
		QNetworkReply* createRequest (Operation,
				const QNetworkRequest&, QIODevice*);
	private:
		void SaveCookies () const;
	signals:
		void requestCreated (QNetworkAccessManager::Operation,
				const QNetworkRequest&, QNetworkReply*);
		void acceptableLanguagesChanged ();

		void hookNAMCreateRequest (LC::IHookProxy_ptr proxy,
					QNetworkAccessManager *manager,
					QNetworkAccessManager::Operation *op,
					QIODevice **dev);
	};
};
