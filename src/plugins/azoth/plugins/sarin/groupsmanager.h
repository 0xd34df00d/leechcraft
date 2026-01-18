/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include <util/sll/void.h>
#include "types.h"

namespace LC::Azoth::Sarin
{
	class GroupChatEntry;
	class ToxAccount;
	class ToxRunner;

	class GroupsManager : public QObject
	{
		Q_OBJECT

		ToxAccount& Acc_;

		QHash<uint32_t, GroupChatEntry*> Groups_;
	public:
		explicit GroupsManager (ToxAccount&);

		ToxAccount& GetAccount ();

		void HandleToxThreadChanged (const std::shared_ptr<ToxRunner>&);

		using JoinResult = Util::Either<JoinGroupError, Util::Void>;
		Util::ContextTask<JoinResult> Join (QString groupId, QString nick, QString password);

		void HandleLeft (uint32_t groupNum);
	};
}
