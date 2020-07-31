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
	class VisitorTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testBasicVisitor ();
		void testBasicVisitorGenericFallback ();
		void testBasicVisitorCoercion ();
		void testBasicVisitorCoercionGenericFallback ();

		void testNonCopyableFunctors ();

		void testAcceptsRValueRef ();

		void testLValueRef ();
		void testLValueRef2 ();

		void testPrepareVisitor ();
		void testPrepareVisitorConst ();
		void testPrepareVisitorRValue ();
		void testPrepareVisitorFinally ();
		void testPrepareJustAutoVisitor ();
		void testPrepareRecursiveVisitor ();
		void testPrepareVisitorMutable ();
	};
}
}
