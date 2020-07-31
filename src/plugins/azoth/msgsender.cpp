/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "msgsender.h"
#include <memory>
#include <QMessageBox>
#include <util/xpc/defaulthookproxy.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/irichtextmessage.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	MsgSender::MsgSender (ICLEntry *e, IMessage::Type type, QString text, QString variant, QString richText)
	{
		deleteLater ();

		Core::Instance ().RegisterHookable (this);

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();

		// TODO pass type without casts
		emit hookMessageWillCreated (proxy, this, e->GetQObject (), static_cast<int> (type), variant);
		if (proxy->IsCancelled ())
			return;

		int intType = static_cast<int> (type);
		proxy->FillValue ("type", intType);
		type = static_cast<IMessage::Type> (intType);
		proxy->FillValue ("variant", variant);
		proxy->FillValue ("text", text);

		const auto msg = e->CreateMessage (type, variant, text);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to create message to"
					<< e->GetEntryID ();
			return;
		}
		const auto richMsg = qobject_cast<IRichTextMessage*> (msg->GetQObject ());
		if (richMsg &&
				!richText.isEmpty ())
			richMsg->SetRichBody (richText);

		proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookMessageCreated (proxy, this, msg->GetQObject ());
		if (proxy->IsCancelled ())
			return;

		try
		{
			msg->Send ();
		}
		catch (const std::exception& ex)
		{
			qWarning () << Q_FUNC_INFO
					<< "error sending message to"
					<< e->GetEntryID ()
					<< e->GetEntryName ()
					<< variant
					<< ex.what ();

			throw;
		}
	}
}
}
