/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cryptosystemtest.h"
#include <cstdlib>
#include <QtTest/QtTest>
#include <QByteArray>
#include <QList>
#include "../cryptosystem.h"
#include "../ciphertextformat.h"

using namespace LC::Plugins::SecMan::StoragePlugins::SecureStorage;

QTEST_MAIN (CryptoSystemTest)

namespace
{

	/**
	 * Check whether all corresponding field in two ciphertexts are different.
	 * @param ctf1 first ciphertext.
	 * @param ctf2 second ciphertext.
	 * @return true if all corresponding field in ciphertexts are different.
	 */
	bool AllFieldsDifferent (const CipherTextFormat& ctf1, const CipherTextFormat& ctf2)
	{
		if (!memcmp (ctf1.Iv (), ctf2.Iv (), IVLength))
			return false;
		if (!memcmp (ctf1.Rnd (), ctf2.Rnd (), RndLength))
			return false;
		if (!memcmp (ctf1.Hmac (), ctf2.Hmac (), HMACLength))
			return false;
		if (ctf1.GetDataLength () == ctf2.GetDataLength ())
			if (ctf1.GetDataLength () != 0)
				if (!memcmp (ctf1.Data (), ctf2.Data (), ctf1.GetDataLength ()))
					return false;
		return true;
	}

	/**
	 * Checks whether all ciphertexts in list have different 
	 * corresponding fields.
	 * @param list list of ciphertexts.
	 * @return true if all ciphertexts in list have different 
	 * corresponding fields.
	 */
	bool AllDiffer (const QList<QByteArray>& list)
	{
		QList<CipherTextFormat> ctfs;
		for (const auto& a : list)
			ctfs << CipherTextFormat (const_cast<char*> (a.data ()),
					CipherTextFormatUtils::DataLengthFor (a.size ()));
		for (int i = 0, len = ctfs.length (); i < len; ++i)
			for (int j = i + 1; j < len; ++j)
				if (!AllFieldsDifferent (ctfs.at (i), ctfs.at (j)))
					return false;
		return true;
	}

	/**
	 * Encrypt all data by all cryptosystems.
	 * @param css list of cryptosystems.
	 * @param datas list of texts to encrypt.
	 * @return list of ciphertexts with length (css.size () * datas.size ())
	 */
	QList<QByteArray> allCipherTexts (const QList<CryptoSystem*>& css, const QList<QByteArray*>& datas)
	{
		QList<QByteArray> cipherTexts;
		for (const auto cs : css)
			for (const auto data : datas)
				cipherTexts << cs->Encrypt (*data);
		return cipherTexts;
	}

	/**
	 * Generate random array with random contents and random length
	 * (from minLength to maxLength).
	 * 
	 * If maxLength ≤ minLength, the generated array will have
	 * length specified by minLength parameter.
	 * 
	 * @param minLength minimum length (including).
	 * @param maxLength maximum length (excluding).
	 * @return array with random data.
	 */
	QByteArray randomData (unsigned minLength, unsigned maxLength = 0)
	{
		unsigned length;
		if (maxLength <= minLength)
			length = minLength;
		else
			length = minLength + qrand () % (maxLength - minLength);

		QByteArray result;
		for (unsigned i = 0; i < length; i++)
			result.append (qrand ());
		return result;
	}

	void TestEncryptDecryptCorrect (const QString& password, const QByteArray& data)
	{
		CryptoSystem cs1 (password);
		CryptoSystem cs2 (password);
		QByteArray enc1 = cs1.Encrypt (data);
		QByteArray enc2 = cs2.Encrypt (data);
		const QByteArray& dec12 = cs1.Decrypt (enc2);
		const QByteArray& dec21 = cs2.Decrypt (enc1);
		QVERIFY (dec12 == dec21);
		CipherTextFormat ctf1 (enc1.data (), CipherTextFormatUtils::DataLengthFor (enc1.length ()));
		CipherTextFormat ctf2 (enc2.data (), CipherTextFormatUtils::DataLengthFor (enc2.length ()));
		QVERIFY (AllFieldsDifferent (ctf1, ctf2));
	}
}

void CryptoSystemTest::initTestCase ()
{
	qsrand (time (0));
}

void CryptoSystemTest::testHash ()
{
	CryptoSystem cs1 ("pass");
	CryptoSystem cs2 ("");

	QVERIFY (cs1.Hash (QByteArray ("")).toHex () ==
			QByteArray ("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

	QVERIFY (cs1.Hash (QByteArray ("")).toHex () !=
			QByteArray ("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b854"));

	QCOMPARE (cs2.Hash (QByteArray ("test")).toHex (),
			QByteArray ("9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08"));

	QCOMPARE (cs1.Hash (QByteArray ("rJklspsjsHKS973h0w3jH(Y30-s*sdf0249sdf-k3-492j")).toHex (),
			QByteArray ("bc6f51ff3f474d205f5f6f764278e86012a2ead9f06e47283856fc746e874981"));
}

void CryptoSystemTest::testDifferentCipherText ()
{
	CryptoSystem cs1 ("yUnsikosdfh");
	CryptoSystem cs2 ("yUnsikosdfh");
	CryptoSystem cs3 ("pass");
	CryptoSystem cs4 ("pass");

	QList<CryptoSystem*> css;
	css << &cs1 << &cs2 << &cs3 << &cs4 << &cs1 << &cs3;

	QByteArray data1 ("test-data");
	QByteArray data2 ("rJklspsjsHKS973h0w3jH(Y30-s*sdf0249sdf-k3-492j");
	QByteArray data3;

	QList<QByteArray*> datas;
	datas << &data1 << &data2 << &data3 << &data3 << &data1;

	const QList<QByteArray>& cipherTexts = allCipherTexts (css, datas);

	QVERIFY (AllDiffer (cipherTexts));
}

#define EXPECT_EXCEPTION(statement, exception) \
	try { \
		statement; \
		QFAIL (#exception " must be throwed"); \
	} catch (exception&) {}

void CryptoSystemTest::testEncryptDecrypt ()
{
	CryptoSystem cs1 ("qwerty");
	CryptoSystem cs2 ("qwerty");
	CryptoSystem cs3 ("uiop");

	QList<QByteArray*> datas;
	QByteArray data1;
	QByteArray data2;
	data2.fill (0, IVLength + 1);
	QByteArray data3 = QByteArray::fromHex ("bc6f51ff3f474d205f5f6f764278e86012a2ead9f06e47283856fc746e874981");
	datas << &data1 << &data2 << &data3;

	for (const auto data : datas)
	{
		QByteArray e1 = cs1.Encrypt (*data);
		QByteArray e2 = cs2.Encrypt (*data);
		QByteArray e3 = cs3.Encrypt (*data);

		// check correct decryption
		try
		{
			QVERIFY (cs1.Decrypt (e1) == *data);
			QVERIFY (cs2.Decrypt (e1) == *data);
			QVERIFY (cs2.Decrypt (e2) == *data);

			QVERIFY (cs3.Decrypt (e3) == *data);
			QVERIFY (cs3.Decrypt (e3) == cs3.Decrypt (e3));
		}
		catch (WrongHMACException& e)
		{
			QFAIL ("Wrong HMAC");
		}

		// check decryption with wrong password (WrongHMACException must be throwed)
		EXPECT_EXCEPTION (cs1.Decrypt (e3), WrongHMACException);
		EXPECT_EXCEPTION (cs2.Decrypt (e3), WrongHMACException);
		EXPECT_EXCEPTION (cs3.Decrypt (e1), WrongHMACException);
		EXPECT_EXCEPTION (cs3.Decrypt (e2), WrongHMACException);
	}
}

void CryptoSystemTest::testEncryptDecryptRandom ()
{
	const int nTests = 10000;
	for (int i = 0; i < nTests; ++i)
	{
		const QString password (randomData (0, 100));
		const QByteArray& data = randomData (0, 16384);
		TestEncryptDecryptCorrect (password, data);
	}
}

void CryptoSystemTest::testEncryptDecryptLength ()
{
	const int maxLength = 1025;
	for (int len = 0; len <= maxLength; ++len)
	{
		const QString password (randomData (len));
		const QByteArray& data = randomData (len);
		TestEncryptDecryptCorrect (password, data);
	}
}

void CryptoSystemTest::testDecryptShortCipherText ()
{
	int minLength = CipherTextFormatUtils::BufferLengthFor (0);
	CryptoSystem cs ("password");
	for (int i = 0; i < minLength; i++)
	{
		QByteArray array;
		array.resize (i);
		EXPECT_EXCEPTION (cs.Decrypt (array), WrongHMACException);
	}
}
