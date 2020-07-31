/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <stdexcept>
#include <memory>
#include <QByteArray>
#include <QString>

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	class WrongHMACException : public std::exception
	{
	public:
		WrongHMACException () { }
		const char* what () const throw ();
	};

	class CryptoSystem
	{
		friend class CryptoSystemTest;
		QByteArray Key_;
	public:
		CryptoSystem (const QString &password);
		~CryptoSystem ();
		QByteArray Encrypt (const QByteArray& data) const;
		QByteArray Decrypt (const QByteArray& cipherText) const;
	private:
		QByteArray Hash (const QByteArray& data) const;
		QByteArray CreateKey (const QString& password) const;
	};

	typedef std::shared_ptr<CryptoSystem> CryptoSystem_ptr;
}
}
}
