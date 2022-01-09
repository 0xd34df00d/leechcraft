/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

class QNetworkAccessManager;

namespace LC::Util
{
	struct ReplyWithHeaders;
}

namespace LC::Poshuku::CleanWeb
{
	class PslHandler;
	class PslFetcher : public QObject
	{
		const QString PslPath_;

		QNetworkAccessManager& NAM_;

		std::unique_ptr<PslHandler> Handler_;
	public:
		explicit PslFetcher (QNetworkAccessManager&, QObject* = nullptr);
		~PslFetcher () override;

		const PslHandler& GetPsl () const;
	private:
		void CheckRefresh ();
		void HandleReply (const QDateTime&, const Util::ReplyWithHeaders&);
	};
}
