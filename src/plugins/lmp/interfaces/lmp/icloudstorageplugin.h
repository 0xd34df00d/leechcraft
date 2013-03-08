/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QString;
class QObject;
class QIcon;

namespace LeechCraft
{
namespace LMP
{
	enum class CloudStorageError
	{
		NoError,
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
		virtual ~ICloudStoragePlugin () {}

		virtual QObject* GetObject () = 0;

		virtual QString GetCloudName () const = 0;

		virtual QIcon GetCloudIcon () const = 0;

		virtual QStringList GetSupportedFileFormats () const = 0;

		virtual void Upload (const QString& account, const QString& localPath) = 0;

		virtual QStringList GetAccounts () const = 0;
	protected:
		virtual void uploadFinished (const QString& localPath,
				CloudStorageError, const QString& errorStr) = 0;

		virtual void accountsChanged () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::LMP::ICloudStoragePlugin, "org.LeechCraft.LMP.ICloudStoragePlugin/1.0");
