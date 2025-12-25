/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include "interfaces/lmp/isyncplugin.h"
#include "transcoder.h"

namespace LC::LMP
{
	class SyncManager : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		Util::ContextTask<void> RunUpload (ISyncPlugin*, QString mount, QStringList, TranscodingParams);
	private:
		Util::ContextTask<void> UploadTranscoded (Transcoder::Result result,
				ISyncPlugin *syncer, QString mount, TranscodingParams params);
	};
}
