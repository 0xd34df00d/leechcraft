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
#include <QInputDialog>
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

		const auto makeErrorNotification = [=, this] (const QString& text)
		{
			auto e = Util::MakeNotification ("Imgaste", text, Priority::Critical);
			const auto nah = new Util::NotificationActionHandler { e };
			const auto guard = connect (nah,
					&QObject::destroyed,
					this,
					&QObject::deleteLater);
			nah->AddFunction (tr ("Try another service..."),
					[=, this]
					{
						disconnect (guard);
						TryAnotherService (serviceName);
					});
			em->HandleEntity (e);
		};

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
						[=] (const SingleServiceUploader::NetworkRequestError& error)
						{
							qWarning () << Q_FUNC_INFO
									<< "original URL:"
									<< error.OriginalUrl_
									<< error.NetworkError_
									<< error.HttpCode_.value_or (-1)
									<< error.ErrorString_;

							const auto& text = tr ("Image upload failed: %1")
									.arg (error.ErrorString_);
							makeErrorNotification (text);
						},
						[=] (const SingleServiceUploader::ServiceAPIError&)
						{
							qWarning () << Q_FUNC_INFO
									<< serviceName;

							const auto& text = tr ("Image upload to %1 failed: service error.")
									.arg ("<em>" + serviceName + "</em>");
							makeErrorNotification (text);
						}
					}
				};
	}

	void Uploader::TryAnotherService (const QString& failedService)
	{
		const ImageInfo info { .Size_ = static_cast<quint64> (Data_.size ()), .Dim_ = {} };

		QStringList otherServices;
		for (const auto& service : GetAllServices ())
		{
			if (!service->Accepts (info))
				continue;

			const auto& name = service->GetName ();
			if (name != failedService)
				otherServices << name;
		}

		bool ok = false;
		const auto& serviceName = QInputDialog::getItem (nullptr,
				"Imgaste",
				tr ("Please select another service to try:"),
				otherServices,
				0,
				false,
				&ok);
		if (!ok || serviceName.isEmpty ())
		{
			deleteLater ();
			return;
		}

		Upload (serviceName);
	}
}
