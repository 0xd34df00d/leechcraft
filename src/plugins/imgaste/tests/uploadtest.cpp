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
#include <QtTest>
#include <util/sll/either.h>
#include <util/threads/coro.h>
#include <util/threads/coro/getresult.h>
#include "hostingservice.h"

Q_DECLARE_METATYPE (LC::Imgaste::HostingService)

QTEST_GUILESS_MAIN (LC::Imgaste::UploadTest)

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

		Util::Task<Util::NetworkResult> Upload (HostingService service, QNetworkAccessManager& nam)
		{
			co_return co_await *Post (service, GetImageContents (), Format::PNG, nam);
		}
	}

	void UploadTest::testUpload_data ()
	{
		if (!qEnvironmentVariableIntValue ("LC_TEST_EXTERNAL_SERVICES"))
			QSKIP ("uploads to the real hosting services, set LC_TEST_EXTERNAL_SERVICES=1 to run");

		QTest::addColumn<HostingService> ("service");

		for (const auto& service : GetAllServices ())
			QTest::newRow (qPrintable (service.Name_)) << service;
	}

	void UploadTest::testUpload ()
	{
		QFETCH (const HostingService, service);

		QNetworkAccessManager nam;
		nam.setTransferTimeout ();
		const auto& result = Util::GetTaskResult (Upload (service, nam));
		if (const auto error = result.IsError ())
			QFAIL (qPrintable (error->ErrorText_));

		const auto& link = service.GetLink_ (result.GetReplyData ());
		QVERIFY2 (link.IsRight (), result.GetReplyData ().constData ());
		qDebug () << link.GetRight ();
	}
}
