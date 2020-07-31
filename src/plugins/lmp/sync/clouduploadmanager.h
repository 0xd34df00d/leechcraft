/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "syncmanagerbase.h"

namespace LC
{
namespace LMP
{
	class ICloudStoragePlugin;
	class CloudUploader;

	class CloudUploadManager : public SyncManagerBase
	{
		Q_OBJECT

		QMap<ICloudStoragePlugin*, CloudUploader*> Cloud2Uploaders_;

		struct CloudUpload
		{
			ICloudStoragePlugin *Cloud_;
			QString Account_;
		};
		QMap<QString, CloudUpload> Source2Params_;
	public:
		CloudUploadManager (QObject* = 0);

		void AddFiles (ICloudStoragePlugin*, const QString&, const QStringList&, const TranscodingParams&);
	private:
		void CreateUploader (ICloudStoragePlugin*);
	private slots:
		void handleFileTranscoded (const QString& from, const QString&, QString);
	};
}
}
