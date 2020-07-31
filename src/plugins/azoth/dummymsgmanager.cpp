/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dummymsgmanager.h"
#include "coremessage.h"

namespace LC
{
namespace Azoth
{
	DummyMsgManager::DummyMsgManager ()
	{
	}

	DummyMsgManager& DummyMsgManager::Instance ()
	{
		static DummyMsgManager d;
		return d;
	}

	void DummyMsgManager::AddMessage (CoreMessage *msg)
	{
		Messages_ [msg->OtherPart ()] << msg;
	}

	void DummyMsgManager::ClearMessages (QObject *entry)
	{
		qDeleteAll (Messages_.take (entry));
	}

	QList<IMessage*> DummyMsgManager::GetIMessages (QObject *entry) const
	{
		QList<IMessage*> result;

		for (const auto msgObj : Messages_.value (entry))
			result << qobject_cast<IMessage*> (msgObj);

		return result;
	}

	void DummyMsgManager::entryDestroyed ()
	{
		ClearMessages (sender ());
	}
}
}
