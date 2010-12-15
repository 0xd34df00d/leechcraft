/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include <QObject>
#include <QtTest>

#define VERSIONCOMPARATOR_DEBUG
#include "../versioncomparator.h"

using namespace LeechCraft::Plugins::LackMan;

class TestVersionComparator : public QObject
{
	Q_OBJECT
private slots:
	void doSanityChecks ()
	{
		QCOMPARE (IsVersionLess ("0", "1"), true);
		QCOMPARE (IsVersionLess ("1", "1"), false);
		QCOMPARE (IsVersionLess ("1", "0"), false);
		QCOMPARE (IsVersionLess ("1.0", "0.1"), false);
		QCOMPARE (IsVersionLess ("0.1.0", "0.1.0"), false);
		QCOMPARE (IsVersionLess ("0.1.0", "0.1.1"), true);
		QCOMPARE (IsVersionLess ("0.1.1", "0.1.0"), false);
		QCOMPARE (IsVersionLess ("0.3.70", "0.3.71"), true);
		QCOMPARE (IsVersionLess ("0.3", "0.3.0"), false);
		QCOMPARE (IsVersionLess ("0.3.0", "0.3"), false);
		QCOMPARE (IsVersionLess ("0.3.9999", "0.4"), true);
		QCOMPARE (IsVersionLess ("0.9999.9999.9999.9999", "1"), true);
		QCOMPARE (IsVersionLess ("0.3.70-beta1", "0.3.70"), true);
		QCOMPARE (IsVersionLess ("0.3.70", "0.3.70-beta1"), false);
		QCOMPARE (IsVersionLess ("0.3.70-alpha1", "0.3.70-alpha2"), true);
		QCOMPARE (IsVersionLess ("0.3.70-alpha3", "0.3.70-beta1"), true);
		QCOMPARE (IsVersionLess ("0.3.70-beta1", "0.3.70-alpha3"), false);
		QCOMPARE (IsVersionLess ("0.3.70-beta0", "0.3.70-alpha9"), false);
		QCOMPARE (IsVersionLess ("0.3.70-rc1", "0.3.71"), true);
		QCOMPARE (IsVersionLess ("0.3.71", "0.3.70-rc1"), false);
		QCOMPARE (IsVersionLess ("0.3.70-beta1", "0.4.0.92"), true);
		QCOMPARE (IsVersionLess ("0.4.0.92", "0.3.70-beta1"), false);
		//QCOMPARE (IsVersionLess ("", ""), );
	}

	void perfSimple1 ()
	{
		QBENCHMARK (IsVersionLess ("0", "1"));
	}

	void perfSimple2 ()
	{
		QBENCHMARK (IsVersionLess ("0.1.0", "0.1.1"));
	}

	void perfSimple3 ()
	{
		QBENCHMARK (IsVersionLess ("0.3.70", "0.4"));
	}

	void perfSimple4 ()
	{
		QBENCHMARK (IsVersionLess ("0.9999.9999.9999.9999", "1"));
	}

	void perfAlpha1 ()
	{
		QBENCHMARK (IsVersionLess ("0.3.70", "0.3.70-beta1"));
	}

	void perfAlpha2 ()
	{
		QBENCHMARK (IsVersionLess ("0.3.70-alpha1", "0.3.70-alpha2"));
	}
};
