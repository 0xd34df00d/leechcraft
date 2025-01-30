/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QMap>
#include <QSet>
#include <interfaces/lmp/icloudstorageplugin.h>

namespace LC::LMP::MP3Tunes
{
	class AuthManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::MP3Tunes::AuthManager)

		QMap<QString, QString> Login2Sid_;
		QSet<QString> FailedAuth_;
	public:
		using QObject::QObject;

		using ResultType = Util::Either<ICloudStoragePlugin::UploadError, QString>;

		Util::ContextTask<ResultType> GetSID (const QString&);
	};
}
