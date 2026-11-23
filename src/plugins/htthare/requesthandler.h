/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vector>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <QByteArray>
#include <QUrl>
#include <QMap>
#include <QCoreApplication>

class QFileInfo;

namespace LC
{
namespace HttHare
{
	class StorageManager;
	class IconResolver;
	class TrManager;

	class RequestHandler
	{
		Q_DECLARE_TR_FUNCTIONS (LC::HttHare::RequestHandler)

		boost::asio::ip::tcp::socket& Socket_;
		const StorageManager& StorageMgr_;
		IconResolver& IconResolver_;
		TrManager& TrManager_;

		QUrl Url_;
		QMap<QString, QString> Headers_;

		QByteArray ResponseLine_;
		QList<std::pair<QByteArray, QByteArray>> ResponseHeaders_;
		QByteArray CookedRH_;
		QByteArray ResponseBody_;

		enum class Verb
		{
			Get,
			Head
		};
	public:
		RequestHandler (boost::asio::ip::tcp::socket&, const StorageManager&, IconResolver&, TrManager&);

		boost::asio::awaitable<void> operator() (QByteArray);
	private:
		QString Tr (const char*);

		boost::asio::awaitable<void> ErrorResponse (int, QByteArray, QByteArray = {});
		boost::asio::awaitable<QByteArray> MakeDirResponse (QFileInfo, QString, QUrl);

		boost::asio::awaitable<void> HandleRequest (Verb);
		boost::asio::awaitable<void> WriteDir (QString, QFileInfo, Verb);
		boost::asio::awaitable<void> WriteFile (QString, QFileInfo, Verb);
		boost::asio::awaitable<void> DefaultWrite (Verb);
		std::vector<boost::asio::const_buffer> ToBuffers (Verb);
	};
}
}
