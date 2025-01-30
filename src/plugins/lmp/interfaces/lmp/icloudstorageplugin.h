/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <util/sll/eitherfwd.h>
#include <util/sll/void.h>
#include <util/threads/coro/taskfwd.h>

class QString;
class QObject;
class QIcon;

namespace LC::LMP
{
	enum class CloudStorageError
	{
		LocalError,
		NetError,
		FileHashMismatch,
		InvalidSession,
		NotAuthorized,
		UnsupportedFileFormat,
		FilesizeExceeded,
		StorageFull,
		ServiceError,
		OtherError
	};

	class ICloudStoragePlugin
	{
	public:
		virtual ~ICloudStoragePlugin () = default;

		virtual QObject* GetQObject () = 0;

		virtual QString GetCloudName () const = 0;

		virtual QIcon GetCloudIcon () const = 0;

		virtual QStringList GetSupportedFileFormats () const = 0;

		struct UploadError
		{
			CloudStorageError Code_;
			QString Message_;

			explicit operator QString () const
			{
				return Message_;
			}
		};

		using UploadResult = Util::Either<UploadError, Util::Void>;

		virtual Util::ContextTask<UploadResult> Upload (const QString& account, const QString& localPath) = 0;

		virtual QStringList GetAccounts () const = 0;
	protected:
		virtual void accountsChanged () = 0;
	};
}

Q_DECLARE_INTERFACE (LC::LMP::ICloudStoragePlugin, "org.LeechCraft.LMP.ICloudStoragePlugin/1.0")
