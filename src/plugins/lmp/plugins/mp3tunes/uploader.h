/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/lmp/icloudstorageplugin.h>
#include "authmanager.h"

namespace LC::LMP
{
	enum class CloudStorageError;
}

namespace LC::LMP::MP3Tunes
{
	class AuthManager;

	class Uploader : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::MP3Tunes::Uploader)

		const QString Login_;
		AuthManager *AuthMgr_;
	public:
		Uploader (const QString&, AuthManager*, QObject* = nullptr);

		Util::ContextTask<ICloudStoragePlugin::UploadResult> Upload (const QString&);
	signals:
		void removeThis (const QString&);
	};
}
