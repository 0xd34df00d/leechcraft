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
	class TagsManagerTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testFreshStorageGetsUntagged ();
		void testStoredUntaggedIsReused ();
		void testIdsAreBracedUuidStrings ();
		void testGetIdCreatesAndPersists ();
		void testGetIdTrims ();
		void testGetIdRefusesInvalidNames_data ();
		void testGetIdRefusesInvalidNames ();
		void testGetIdsSkipsRefusedNames ();
		void testDuplicateNamesResolveToTheLowestId ();
		void testStoredNamesAreRepaired_data ();
		void testStoredNamesAreRepaired ();
		void testUnrepairableStoredNamesAreKept_data ();
		void testUnrepairableStoredNamesAreKept ();
		void testRenameRefusals_data ();
		void testRenameRefusals ();
		void testRenameWithoutChange ();
		void testRename ();
		void testRemove ();
		void testModelRowsFollowGetAllTags ();
		void testJoinAndSplit ();
		void testTagIdListIsARegisteredMetaType ();
	};
}
