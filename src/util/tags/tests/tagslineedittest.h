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
	class TagsLineEditTest : public QObject
	{
		Q_OBJECT
	private slots:
		void testSelectorChoiceIsReported ();
		void testSelectorFollowsNonCanonicalText ();
		void testSetTagsSyncsTheSelectorWithBlockedSignals ();
		void testNestedSetTagsWins ();
		void testUnchangedTagsAreNotReported ();
		void testSetTagsIsNeverReported ();
		void testHandEditIsReportedOnce ();
		void testUpdateTagsKeepsAnEditInProgress ();
		void testCompletionReplacesTheTokenUnderCursor_data ();
		void testCompletionReplacesTheTokenUnderCursor ();
		void testSelectorFollowsCompletionModel ();
	};
}
