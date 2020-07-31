/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QNetworkAccessManager>

namespace LC::Snails
{
	struct MessagePageContext;

	class MailWebPageNAM : public QNetworkAccessManager
	{
	public:
		using ContextGetter = std::function<MessagePageContext ()>;
	private:
		ContextGetter CtxGetter_;
	public:
		MailWebPageNAM (ContextGetter, QObject* = nullptr);
	protected:
		QNetworkReply* createRequest (QNetworkAccessManager::Operation, const QNetworkRequest&, QIODevice*);
	private:
		QNetworkReply* HandleNetworkRequest (QNetworkAccessManager::Operation, const QNetworkRequest&, QIODevice*);
		QNetworkReply* HandleCIDRequest (QNetworkAccessManager::Operation, const QNetworkRequest&, QIODevice*);
	};
}
