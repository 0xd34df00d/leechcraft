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
namespace Azoth
{
namespace MuCommands
{
	class PresenceCommandTest : public QObject
	{
		Q_OBJECT
	private slots:
		void accStateChange ();
		void allAccStateChange ();
		void noAccStateChange ();
		void accAlmostCustomStateChange ();
		void accCustomStateChange ();
		void accStatusChange ();
		void accStatusChangeSameLine ();
		void accStatusClear ();
		void accStatusChangeClearSubstr ();
		void accMessageOnly ();
		void accMultilineMessageOnly ();

		void chatPrStateChangeNoNL ();
		void chatPrAlmostCustomStateChangeNoNL ();
		void chatPrCustomStateChangeNoNL ();
		void chatPrStatusChangeNoNL ();
		void chatPrStatusChangeSameLineNoNL ();
	};
}
}
}
