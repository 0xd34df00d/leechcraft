/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploader.h"
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <util/threads/futures.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/icoreproxy.h>
#include "hostingservice.h"
#include "singleserviceuploader.h"

namespace LC::Imgaste
{
	Uploader::Uploader (QByteArray data,
			QString format,
			DataFilterCallback_f callback,
			QStandardItemModel *reprModel)
	: Data_ { std::move (data) }
	, Format_ { std::move (format) }
	, Callback_ { std::move (callback) }
	, ReprModel_ { reprModel }
	{
	}

	namespace
	{
		const HostingService* FromString (const QString& name)
		{
			for (const auto& service : GetAllServices ())
				if (service->GetName () == name)
					return service.get ();

			return nullptr;
		}
	}

	void Uploader::Upload (const QString& serviceName)
	{
		const auto service = FromString (serviceName);

		if (!service)
		{
			QMessageBox::critical (nullptr,
					"LeechCraft",
					tr ("Unknown upload service: %1.")
						.arg (serviceName));
			return;
		}

		const auto em = GetProxyHolder ()->GetEntityManager ();

		auto uploader = new SingleServiceUploader (*service,
				Data_,
				Format_,
				ReprModel_);
		Util::Sequence (this, uploader->GetFuture ()) >>
				Util::Visitor
				{
					[this, em] (const QString& url)
					{
						if (!Callback_)
						{
							QApplication::clipboard ()->setText (url, QClipboard::Clipboard);

							auto text = tr ("Image pasted: %1, the URL was copied to the clipboard")
									.arg ("<em>" + url + "</em>");
							em->HandleEntity (Util::MakeNotification ("Imgaste", text, Priority::Info));
						}
						else
							Callback_ (url);

						deleteLater ();
					},
					Util::Visitor
					{
						[this, em] (const SingleServiceUploader::NetworkRequestError& error)
						{
							qWarning () << Q_FUNC_INFO
									<< "original URL:"
									<< error.OriginalUrl_
									<< error.NetworkError_
									<< error.HttpCode_.value_or (-1)
									<< error.ErrorString_;

							const auto& text = tr ("Image upload failed: %1")
									.arg (error.ErrorString_);
							em->HandleEntity (Util::MakeNotification ("Imgaste", text, Priority::Critical));

							deleteLater ();
						},
						[=] (const SingleServiceUploader::ServiceAPIError&)
						{
							qWarning () << Q_FUNC_INFO
									<< serviceName;

							const auto& text = tr ("Image upload to %1 failed: service error.")
									.arg ("<em>" + serviceName + "</em>");
							em->HandleEntity (Util::MakeNotification ("Imgaste", text, Priority::Critical));
							deleteLater ();
						}
					}
				};
	}
}
