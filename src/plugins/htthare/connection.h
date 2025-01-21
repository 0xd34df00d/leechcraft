/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <boost/asio.hpp>

namespace LC
{
namespace HttHare
{
	class StorageManager;
	class IconResolver;
	class TrManager;

	class Connection : public std::enable_shared_from_this<Connection>
	{
		boost::asio::io_context::strand Strand_;
		boost::asio::ip::tcp::socket Socket_;

		const StorageManager& StorageMgr_;
		IconResolver * const IconResolver_;
		TrManager * const TrManager_;

		boost::asio::streambuf Buf_;
	public:
		Connection (boost::asio::io_context&, const StorageManager&, IconResolver*, TrManager*);

		Connection (const Connection&) = delete;
		Connection& operator= (const Connection&) = delete;

		boost::asio::ip::tcp::socket& GetSocket ();
		boost::asio::io_context::strand& GetStrand ();
		IconResolver* GetIconResolver () const;
		TrManager* GetTrManager () const;

		const StorageManager& GetStorageManager () const;

		void Start ();
	private:
		void HandleHeader (const boost::system::error_code&, unsigned long);
	};

	typedef std::shared_ptr<Connection> Connection_ptr;
}
}
