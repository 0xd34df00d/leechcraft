/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploadtest.h"
#include <QBuffer>
#include <QCoreApplication>
#include <QImage>
#include <QNetworkAccessManager>
#include <QTest>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/threads/coro.h>
#include <util/threads/coro/getresult.h>
#include "hostingservice.h"

Q_DECLARE_METATYPE (LC::Imgaste::HostingService)

int main (int argc, char *argv [])
{
	if (!qEnvironmentVariableIntValue ("LC_TEST_EXTERNAL_SERVICES"))
	{
		qInfo () << "uploads to the real hosting services, set LC_TEST_EXTERNAL_SERVICES=1 to run";
		return LC_TEST_SKIP_RETURN_CODE;
	}

	QCoreApplication app { argc, argv };
	LC::Imgaste::UploadTest test;
	return QTest::qExec (&test, argc, argv);
}

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
