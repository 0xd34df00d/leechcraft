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
#include <QtTest/QtTest>
#include <QByteArray>

using namespace LeechCraft::Plugins::SecMan::StoragePlugins::SecureStorage;

QTEST_MAIN (CryptoSystemTest)

void CryptoSystemTest::testHash ()
{
	CryptoSystem cs1 ("pass");
	CryptoSystem cs2 ("");
	
	QCOMPARE (cs1.Hash (QByteArray ("")).toHex (),
			QByteArray ("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
	
	QCOMPARE (cs1.Hash (QByteArray ("")).toHex () ==
			QByteArray ("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b854"), false);
	
	QCOMPARE (cs2.Hash (QByteArray("test")).toHex (),
			QByteArray ("9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08"));
	
	QCOMPARE (cs1.Hash (QByteArray ("rJklspsjsHKS973h0w3jH(Y30-s*sdf0249sdf-k3-492j")).toHex (),
			QByteArray ("bc6f51ff3f474d205f5f6f764278e86012a2ead9f06e47283856fc746e874981"));
}

