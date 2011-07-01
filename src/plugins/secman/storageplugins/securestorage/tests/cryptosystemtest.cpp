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

#include "cryptosystemtest.h"
#include "../cryptosystem.h"
#include "../ciphertextformat.h"
#include <QtTest/QtTest>
#include <QByteArray>
#include <QList>
#include <cstdlib>

using namespace LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage;

QTEST_MAIN (CryptoSystemTest)

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

namespace
{

	bool AllFieldsDifferent (const CipherTextFormat &ctf1, const CipherTextFormat &ctf2)
	{
		return memcmp (ctf1.Iv (), ctf2.Iv (), IV_LENGTH) &&
				memcmp (ctf1.Rnd (), ctf2.Rnd (), RND_LENGTH) &&
				memcmp (ctf1.Hmac (), ctf2.Hmac (), HMAC_LENGTH) &&
				(ctf1.DataLength_ == ctf2.DataLength_ ?
				memcmp (ctf1.Data (), ctf2.Data (), ctf1.DataLength_) :
				true);
	}

	bool AllDiffer (const QList<QByteArray> &list)
	{
		QList<CipherTextFormat> ctfs;
		Q_FOREACH (const QByteArray &a, list)
		ctfs << CipherTextFormat (const_cast<char*>(a.data ()),
				CipherTextFormat::DataLengthFor (a.size()));
		Q_FOREACH (const CipherTextFormat &c1, ctfs)
		Q_FOREACH (const CipherTextFormat &c2, ctfs)
		if (!AllFieldsDifferent (c1, c2))
			return false;
		return true;
	}

	/**
	 * Encrypt all data by all cryptosystems.
	 * @param css list of cryptosystems.
	 * @param datas list of texts to encrypt.
	 * @return list of ciphertexts with length (css.size () * datas.size ())
	 */
	QList<QByteArray> allCipherTexts (const QList<CryptoSystem*> &css, const QList<QByteArray*> &datas)
	{
		QList<QByteArray> cipherTexts;
		Q_FOREACH (CryptoSystem* cs, css)
		Q_FOREACH (QByteArray* data, datas)
		cipherTexts << cs->Encrypt (*data);
		return cipherTexts;
	}
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

	QList<QByteArray*> datas;
	datas << &data1 << &data2 << &data1;

	const QList<QByteArray> &cipherTexts = allCipherTexts (css, datas);

	QVERIFY (AllDiffer (cipherTexts));
}

#define EXPECT_EXCEPTION(statement, exception) \
	try { \
		statement; \
		QFAIL (#exception " must be throwed"); \
	} catch (exception &) {}

void CryptoSystemTest::testEncryptDecrypt ()
{
	CryptoSystem cs1 ("qwerty");
	CryptoSystem cs2 ("qwerty");
	CryptoSystem cs3 ("uiop");

	QList<QByteArray*> datas;
	QByteArray data1;
	QByteArray data2;
	data2.fill (0, IV_LENGTH + 1);
	QByteArray data3 = QByteArray::fromHex ("bc6f51ff3f474d205f5f6f764278e86012a2ead9f06e47283856fc746e874981");
	datas << &data1 << &data2 << &data3;

	Q_FOREACH (QByteArray* data, datas)
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
		catch (WrongHMACException &e)
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
	QWARN ("Test was not implemented yet.");
}

