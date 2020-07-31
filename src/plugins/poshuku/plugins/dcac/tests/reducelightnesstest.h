/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "testbase.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	class ReduceLightnessTest : public TestBase
	{
		Q_OBJECT
	private slots:
		void testSSSE3 ();
		void testAVX2 ();

		void benchDefault ();
		void benchSSSE3 ();
		void benchAVX2 ();
	};
}
}
}
