/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "connection.h"
#include <QtDebug>
#include "requesthandler.h"

namespace LC
{
namespace HttHare
{
	Connection::Connection (boost::asio::io_service& service,
			const StorageManager& stMgr, IconResolver *resolver, TrManager *trMgr)
	: Strand_ { service }
	, Socket_ { service }
	, StorageMgr_ (stMgr)
	, IconResolver_ { resolver }
	, TrManager_ { trMgr }
	, Buf_ { 2 * 1024 }
	{
	}

	boost::asio::ip::tcp::socket& Connection::GetSocket ()
	{
		return Socket_;
	}

	boost::asio::io_service::strand& Connection::GetStrand ()
	{
		return Strand_;
	}

	IconResolver* Connection::GetIconResolver () const
	{
		return IconResolver_;
	}

	TrManager* Connection::GetTrManager () const
	{
		return TrManager_;
	}

	const StorageManager& Connection::GetStorageManager () const
	{
		return StorageMgr_;
	}

	void Connection::Start ()
	{
		auto conn = shared_from_this ();
		boost::asio::async_read_until (Socket_,
				Buf_,
				std::string { "\r\n\r\n" },
				Strand_.wrap ([conn] (const boost::system::error_code& ec, ulong transferred)
					{ conn->HandleHeader (ec, transferred); }));
	}

	void Connection::HandleHeader (const boost::system::error_code&, unsigned long transferred)
	{
		QByteArray data;
		data.resize (transferred);

		std::istream istr (&Buf_);
		istr.read (data.data (), transferred);

		RequestHandler { shared_from_this () } (data);
	}
}
}
