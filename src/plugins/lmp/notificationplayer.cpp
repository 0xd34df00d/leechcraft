/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationplayer.h"
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "engine/sourceobject.h"
#include "engine/audiosource.h"
#include "engine/output.h"
#include "engine/path.h"
#include "sourceerrorhandler.h"

namespace LC
{
namespace LMP
{
	NotificationPlayer::NotificationPlayer (const QString& audiofile,
			const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	{
		qDebug () << Q_FUNC_INFO << audiofile;
		auto source = new SourceObject (Category::Notification, this);
		auto out = new Output (this);

		new Path (source, out, this);

		source->SetCurrentSource ({ audiofile });
		source->Play ();

		connect (source,
				SIGNAL (stateChanged (SourceState, SourceState)),
				this,
				SLOT (handleStateChanged (SourceState, SourceState)));

		new SourceErrorHandler { source, proxy->GetEntityManager () };
	}

	void NotificationPlayer::handleStateChanged (SourceState state, SourceState previous)
	{
		qDebug () << Q_FUNC_INFO << static_cast<int> (state) << static_cast<int> (previous);
		switch (state)
		{
		case SourceState::Error:
		case SourceState::Stopped:
			deleteLater ();
			break;
		default:
			break;
		}
	}
}
}
