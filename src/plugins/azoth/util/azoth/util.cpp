/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QTimer>
#include <QWidget>
#include <util/sll/qobjectrefcast.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucprotocol.h>
#include <interfaces/azoth/imucjoinwidget.h>

namespace LC::Azoth
{
	bool IsOnline (State st)
	{
		switch (st)
		{
		case SOffline:
		case SError:
		case SConnecting:
		case SInvalid:
			return false;
		default:
			return true;
		}
	}

	QString StateToString (State st)
	{
		switch (st)
		{
		case SOnline:
			return QObject::tr ("Online");
		case SChat:
			return QObject::tr ("Free to chat");
		case SAway:
			return QObject::tr ("Away");
		case SDND:
			return QObject::tr ("Do not disturb");
		case SXA:
			return QObject::tr ("Not available");
		case SOffline:
			return QObject::tr ("Offline");
		default:
			return QObject::tr ("Error");
		}
	}

	void RejoinMuc (const IMUCEntry& entry)
	{
		const auto acc = dynamic_cast<const ICLEntry&> (entry).GetParentAccount ();
		RejoinMuc (*acc, entry.GetIdentifyingData ());
	}

	void RejoinMuc (IAccount& account, const QVariantMap& identifyingData)
	{
		const auto accObj = account.GetQObject ();
		auto& mucProto = qobject_ref_cast<IMUCProtocol> (accObj);

		auto mucJoinWidget = mucProto.GetMUCJoinWidget ();
		auto& imjw = qobject_ref_cast<IMUCJoinWidget> (mucJoinWidget);
		imjw.AccountSelected (accObj);
		imjw.SetIdentifyingData (identifyingData);

		QTimer::singleShot (1000,
				[mucJoinWidget, &imjw, accObj]
				{
					imjw.Join (accObj);
					mucJoinWidget->deleteLater ();
				});
	}
}
