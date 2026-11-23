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
#include "requesthandler.h"

namespace LC
{
namespace HttHare
{
	namespace ip = boost::asio::ip;

	Server::Server (const QList<std::pair<QString, QString>>& addresses)
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

		for (const auto& acceptor : Acceptors_)
			boost::asio::co_spawn (IoService_, Accept (*acceptor), boost::asio::detached);
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

	namespace
	{
		void LogFailure (const std::exception_ptr& failure)
		{
			if (!failure)
				return;

			try
			{
				std::rethrow_exception (failure);
			}
			catch (const std::exception& e)
			{
				qWarning () << "connection failed:" << e.what ();
			}
			catch (...)
			{
				qWarning () << "connection failed";
			}
		}
	}

	boost::asio::awaitable<void> Server::Accept (ip::tcp::acceptor& acceptor)
	{
		for (;;)
		{
			auto [ec, socket] = co_await acceptor.async_accept (boost::asio::as_tuple (boost::asio::use_awaitable));
			if (ec)
				qWarning () << Q_FUNC_INFO
						<< "cannot accept:"
						<< ec.message ().c_str ();
			else
				boost::asio::co_spawn (IoService_, Serve (std::move (socket)), LogFailure);
		}
	}

	boost::asio::awaitable<void> Server::Serve (ip::tcp::socket socket)
	{
		boost::asio::streambuf buf { 2 * 1024 };
		const auto [ec, size] = co_await boost::asio::async_read_until (socket, buf, std::string_view { "\r\n\r\n" },
				boost::asio::as_tuple (boost::asio::use_awaitable));
		const auto headerTooLong = ec == boost::asio::error::not_found;
		if (ec && !headerTooLong)
			co_return;

		QByteArray header { static_cast<qsizetype> (size), Qt::Uninitialized };
		std::istream { &buf }.read (header.data (), header.size ());

		RequestHandler handler { socket, StorageMgr_, IconResolver_, TrManager_ };
		co_await handler (std::move (header));

		boost::system::error_code shutdownEc;
		socket.shutdown (ip::tcp::socket::shutdown_both, shutdownEc);
	}
}
}
