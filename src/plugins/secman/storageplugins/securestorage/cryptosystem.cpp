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

#include "cryptosystem.h"
#include <QByteArray>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

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
					CryptoSystem::CryptoSystem (const QString &password)
					{
						Key_ = CreateKey (password);
					}

					QByteArray CryptoSystem::Encrypt (const QByteArray &data) const
					{
						QByteArray result;
						result.fill(0, IV_LENGTH + HMAC_LENGTH + data.size());
						const unsigned char* in = reinterpret_cast<const unsigned char*>(data.data ());
						unsigned char* iv = reinterpret_cast<unsigned char*>(result.data ());
						unsigned char* hmac = reinterpret_cast<unsigned char*>(result.data () + IV_LENGTH);
						unsigned char* cipherText = reinterpret_cast<unsigned char*>(result.data () + IV_LENGTH + HMAC_LENGTH);
						
						int length=data.size ();
						return result;
					}

					QByteArray CryptoSystem::Decrypt (const QByteArray &data) const
					{
						return QByteArray ();
					}

					QByteArray CryptoSystem::HMAC (const QByteArray &data) const
					{
						return QByteArray ();
					}

					QByteArray CryptoSystem::Hash (const QByteArray &data) const
					{
						unsigned char hash[HASH_LENGTH];
						SHA256 (reinterpret_cast<const unsigned char*>(data.data ()), data.size (), hash);
						return QByteArray (reinterpret_cast<char*>(hash), HASH_LENGTH);
					}

					QByteArray CryptoSystem::CreateKey (const QString &password) const
					{
						QByteArray res = Hash (password.toUtf8 ());
						res.truncate (KEY_LENGTH);
						return res;
					}
				}
			}
		}
	}
}
