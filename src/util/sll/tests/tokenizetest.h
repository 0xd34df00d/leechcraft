/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::Util
{
	class TokenizeTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testForwardEmpty ();
		void testForwardSimple ();
		void testForwardWithEmpty ();
		void testForwardStartSep ();
		void testForwardEndSep ();
		void testForwardJustSeps ();

		void testReverseEmpty ();
		void testReverseSimple ();
		void testReverseWithEmpty ();
		void testReverseStartSep ();
		void testReverseEndSep ();
		void testReverseJustSeps ();
	};
}
