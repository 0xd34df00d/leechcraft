/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadtest.h"
#include <QBuffer>
#include <QImage>
#include <QNetworkAccessManager>
#include <QtDebug>
#include <util/threads/coro.h>
#include <util/threads/coro/getresult.h>
#include <util/threads/coro/networkresult.h>
#include "hostingservice.h"

namespace LC::Imgaste
{
	namespace
	{
		auto GetImageContents ()
		{
			QImage image { 800, 600, QImage::Format_RGB32 };
			image.fill (Qt::green);

			QByteArray bytes;
			QBuffer buf (&bytes);
			buf.open (QIODevice::ReadWrite);
			image.save (&buf, "PNG");

			return bytes;
		}
	}

	void UploadTest::testUpload ()
	{
		auto task = [] () -> Util::Task<void>
		{
			const auto& contents = GetImageContents ();
			qDebug () << "file size:" << contents.size ();

			QNetworkAccessManager nam;
			for (const auto& service : GetAllServices ())
			{
				qDebug () << "testing" << service->GetName ();
				const auto& result = co_await *service->Post (contents, Format::PNG, &nam);
				if (const auto err = result.IsError ())
					qDebug () << "got error:" << *err;
			}
		} ();

		GetTaskResult (task);
	}
}
