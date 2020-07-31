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
	class OralTest_SimpleRecord : public QObject
	{
		Q_OBJECT
	private slots:
		void testSimpleRecordInsertSelect ();
		void testSimpleRecordInsertReplaceSelect ();
		void testSimpleRecordInsertIgnoreSelect ();

		void testSimpleRecordInsertSelectByPos ();
		void testSimpleRecordInsertSelectByPos2 ();
		void testSimpleRecordInsertSelectByPos3 ();
		void testSimpleRecordInsertSelectOneByPos ();

		void testSimpleRecordInsertSelectByFields ();
		void testSimpleRecordInsertSelectByFields2 ();
		void testSimpleRecordInsertSelectByFields3 ();

		void testSimpleRecordInsertSelectOneByFields ();

		void testSimpleRecordInsertSelectSingleFieldByFields ();
		void testSimpleRecordInsertSelectFieldsByFields ();

		void testSimpleRecordInsertSelectFieldsByFieldsOrderAsc ();
		void testSimpleRecordInsertSelectFieldsByFieldsOrderDesc ();

		void testSimpleRecordInsertSelectFieldsByFieldsOrderManyAsc ();
		void testSimpleRecordInsertSelectFieldsByFieldsOrderManyDesc ();

		void testSimpleRecordInsertSelectNoOffsetLimit ();
		void testSimpleRecordInsertSelectOffsetNoLimit ();
		void testSimpleRecordInsertSelectOffsetLimit ();

		void testSimpleRecordInsertSelectCount ();
		void testSimpleRecordInsertSelectCountByFields ();

		void testSimpleRecordInsertSelectMin ();
		void testSimpleRecordInsertSelectMax ();

		void testSimpleRecordInsertSelectMinPlusMax ();
		void testSimpleRecordInsertSelectValuePlusMinPlusMax ();
		void testSimpleRecordInsertSelectAllPlusMinPlusMax ();

		void testSimpleRecordInsertSelectLike ();

		void testSimpleRecordUpdate ();
		void testSimpleRecordUpdateExprTree ();
		void testSimpleRecordUpdateMultiExprTree ();
	};
}
}
