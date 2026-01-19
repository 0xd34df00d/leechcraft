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

namespace LC::Azoth::Sarin
{
	struct ConfInvitationEvent;
	class ConfEntry;
	class ToxAccount;
	class ToxRunner;

	class ConfsManager : public QObject
	{
		Q_OBJECT

		ToxAccount& Acc_;
		QHash<uint32_t, ConfEntry*> Conf2Entry_;
	public:
		explicit ConfsManager (ToxAccount&);

		ToxAccount& GetAccount ();

		Util::ContextTask<void> Join (QByteArray cookie, uint32_t friendNum, int retry = 0);
		void HandleSelfLeft (uint32_t);
	private:
		void HandleInvited (const ConfInvitationEvent&);
		void HandleToxThreadChanged (const std::shared_ptr<ToxRunner>&);
	};
}
