/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <interfaces/lmp/icloudstorageplugin.h>

namespace LC::LMP
{
	class CloudUploader : public QObject
	{
		Q_OBJECT

		ICloudStoragePlugin *Cloud_;
	public:
		struct UploadJob
		{
			bool RemoveOnFinish_ = false;
			QString Account_;
			QString Filename_;
		};
	private:
		QList<UploadJob> Queue_;
		bool IsRunning_ = false;
	public:
		explicit CloudUploader (ICloudStoragePlugin*, QObject* = nullptr);

		void Upload (const UploadJob&);
	private:
		Util::ContextTask<> DrainQueue ();
	signals:
		void startedCopying (const QString&);
		void finishedCopying ();
	};
}
