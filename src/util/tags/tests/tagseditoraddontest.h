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
	class TagsEditorAddonTest : public QObject
	{
		Q_OBJECT
	public:
		static void initMain ();
	private slots:
		void testSelectorChoiceIsReported ();
		void testSelectorFollowsNonCanonicalText ();
		void testSelectorChoiceKeepsUnknownTokens ();
		void testSetTagsSyncsTheSelectorWithBlockedSignals ();
		void testNestedSetTagsWins ();
		void testUnchangedTagsAreNotReported ();
		void testSetTagsIsNeverReported ();
		void testHandEditIsReportedOnce ();
		void testUpdateTagsKeepsAnEditInProgress ();
		void testCompletionReplacesTheTokenUnderCursor_data ();
		void testCompletionReplacesTheTokenUnderCursor ();
		void testEnterAcceptsTheCompletion ();
		void testCursorMoveRefreshesCompletion ();
		void testInnerWhitespaceIsKept ();
		void testSelectorFollowsTheTagsModel ();
		void testSelectorButtonBindsAPopup ();
		void testOneLiveSelectorAtATime ();
	};
}
