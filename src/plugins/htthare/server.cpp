/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "server.h"
#include <QString>
#include <QtDebug>
#include "connection.h"
#include "iconresolver.h"
#include "trmanager.h"

namespace LC
{
namespace HttHare
{
	namespace ip = boost::asio::ip;

	Server::Server (const QList<QPair<QString, QString>>& addresses)
	: IconResolver_ { new IconResolver  }
	, TrManager_ { new TrManager }
	{
		ip::tcp::resolver resolver { IoService_ };

		for (const auto& pair : addresses)
		{
			try
			{
				const auto endpoints = resolver.resolve (pair.first.toStdString (), pair.second.toStdString ());
				for (const ip::tcp::endpoint endpoint : endpoints)
				{
					auto accPtr = std::make_unique<ip::tcp::acceptor> (IoService_);
					accPtr->open (endpoint.protocol ());
					accPtr->set_option (ip::tcp::acceptor::reuse_address (true));
					accPtr->bind (endpoint);
					accPtr->listen ();

					Acceptors_.emplace_back (std::move (accPtr));
				}
			}
			catch (const std::exception& e)
			{
				qWarning () << "error binding" << pair << e.what ();
			}
		}

		StartAccept ();
	}

	Server::~Server ()
	{
		if (!IoService_.stopped ())
			Stop ();
	}

	void Server::Start ()
	{
		if (Acceptors_.empty ())
			return;

		for (auto i = 0; i < 2; ++i)
			Threads_.emplace_back ([this] { IoService_.run (); });
	}

	void Server::Stop ()
	{
		IoService_.stop ();
		for (auto& thread : Threads_)
			thread.join ();
		Threads_.clear ();
	}

	void Server::StartAccept ()
	{
		Connection_ptr connection { new Connection { IoService_, StorageMgr_, IconResolver_, TrManager_ } };

		for (auto& acceptor : Acceptors_)
			acceptor->async_accept (connection->GetSocket (),
					[this, connection] (const boost::system::error_code& ec)
					{
						if (!ec)
							connection->Start ();
						else
							qWarning () << Q_FUNC_INFO
									<< "cannot accept:"
									<< ec.message ().c_str ();

						StartAccept ();
					});
	}
}
}
