/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <thread>
#include <boost/asio.hpp>
#include "storagemanager.h"

template<typename T>
class QSet;

class QString;

namespace LC
{
namespace HttHare
{
	class IconResolver;
	class TrManager;

	class Server
	{
		boost::asio::io_service IoService_;
		std::vector<std::unique_ptr<boost::asio::ip::tcp::acceptor>> Acceptors_;

		StorageManager StorageMgr_;

		std::vector<std::thread> Threads_;

		IconResolver * const IconResolver_;
		TrManager * const TrManager_;
	public:
		Server (const QList<QPair<QString, QString>>& addresses);
		~Server ();

		Server (const Server&) = delete;
		Server& operator= (const Server&) = delete;

		void Start ();
		void Stop ();
	private:
		void StartAccept ();
	};
}
}
