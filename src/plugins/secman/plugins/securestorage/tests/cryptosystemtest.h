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

#ifndef PLUGINS_SECMAN_SECURESTORAGE_CRYPTOSYSTEM_TEST_H
#define PLUGINS_SECMAN_SECURESTORAGE_CRYPTOSYSTEM_TEST_H
#include <QObject>

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

	class CryptoSystemTest : public QObject
	{
		Q_OBJECT
	private slots:
		/** Initialization. */
		void initTestCase ();
		/** Test SHA-256 hash. */
		void testHash ();
		/** Check that ciphertexts (and HMAC's) of same data are different. */
		void testDifferentCipherText ();
		/** Check that we get the original data after encrypting and decrypting. */
		void testEncryptDecrypt ();
		/** 
		 * Check that we get the original data after encrypting and decrypting 
		 * (with random data and length). 
		 */
		void testEncryptDecryptRandom ();
		/** 
		 * Check that we get the original data after encrypting and decrypting
		 * (with different lengthes and random data). 
		 */
		void testEncryptDecryptLength ();
		/**
		 * Check that cryptosystem operates correctly with too short ciphertexts.
		 */
		void testDecryptShortCipherText ();
	};
}
}
}
}
}
#endif
