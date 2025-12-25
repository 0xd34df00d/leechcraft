/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QFile>
#include <util/sll/either.h>
#include <util/threads/coro/taskfwd.h>

namespace LC::LMP
{
	enum class SyncConfLevel
	{
		None,
		Medium,
		High
	};

	class ISyncPlugin
	{
	public:
		virtual ~ISyncPlugin () = default;

		virtual QObject* GetQObject () = 0;

		virtual QString GetSyncSystemName () const = 0;

		virtual SyncConfLevel CouldSync (const QString& path) = 0;

		struct UploadSuccess {};
		struct UploadFailure
		{
			QFile::FileError Error_;
			QString ErrorStr_;
		};

		using UploadResult = Util::Either<UploadFailure, UploadSuccess>;

		struct UploadJob
		{
			QString LocalPath_;
			QString OriginalLocalPath_;
			QString Target_;
			QString TargetRelPath_;
		};

		virtual Util::ContextTask<UploadResult> Upload (UploadJob uploadJob) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::LMP::ISyncPlugin, "org.LeechCraft.LMP.ISyncPlugin/1.0")
