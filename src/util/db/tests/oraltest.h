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
	class OralTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testAutoPKeyRecordInsertSelect ();
		void testAutoPKeyRecordInsertRvalueReturnsPKey ();
		void testAutoPKeyRecordInsertConstLvalueReturnsPKey ();
		void testAutoPKeyRecordInsertSetsPKey ();

		void testNoPKeyRecordInsertSelect ();

		void testNonInPlaceConstructibleRecordInsertSelect ();

		void testComplexConstraintsRecordInsertSelectDefault ();
		void testComplexConstraintsRecordInsertSelectIgnore ();
		void testComplexConstraintsRecordInsertSelectReplace ();
	};
}
}
