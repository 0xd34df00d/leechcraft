/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_CRYPTOSYSTEM_H
#define PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_CRYPTOSYSTEM_H
#include <stdexcept>
#include <QByteArray>
#include <QString>

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
namespace StoragePlugins
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
}
}
}
}
}

#endif
