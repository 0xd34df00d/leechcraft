/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QObject>
#include <QtTest>

#define VERSIONCOMPARATOR_DEBUG
#include "../versioncomparator.h"

using namespace LC::Plugins::LackMan;

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
