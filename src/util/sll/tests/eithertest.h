/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Util
{
	class EitherTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testBasicLeft ();
		void testBasicRight ();

		void testFMapLeft ();
		void testFMapRight ();
		void testFMapRightChangeType ();

		void testPure ();
		void testGSL ();
		void testGSLLeft ();

		void testBind ();
		void testBindLeft ();

		void testBindLeftNotConstructed ();
	};
}
}
