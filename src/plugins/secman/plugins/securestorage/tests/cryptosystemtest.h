/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_SECMAN_SECURESTORAGE_CRYPTOSYSTEM_TEST_H
#define PLUGINS_SECMAN_SECURESTORAGE_CRYPTOSYSTEM_TEST_H
#include <QObject>

namespace LC
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
