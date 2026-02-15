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
#include <interfaces/ijobholder.h>
#include <util/threads/futures.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/progressmanager.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/icoreproxy.h>
#include "hostingservice.h"

namespace LC::Imgaste
{
	namespace
	{
		struct UploadContext
		{
			QByteArray Data_;
			Format Fmt_;
			Util::ProgressManager& ProgressManager_;
			DataFilterCallback_f Callback_;
			QHash<QString, HostingService> Services_;

			QString ServiceName_;
		};

		void TryAnotherService (UploadContext);

		void NotifyError (const UploadContext& ctx, const QString& text)
		{
			auto e = Util::MakeNotification (PLUGIN_VISIBLE_NAME, text, Priority::Critical);
			const auto nah = new Util::NotificationActionHandler { e };
			nah->AddFunction (QObject::tr ("Try another service..."),
					[=] { TryAnotherService (ctx); });
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		void HandleSuccess (const QString& url, const UploadContext& ctx)
		{
			if (ctx.Callback_)
				ctx.Callback_ (url);
			else
			{
				QApplication::clipboard ()->setText (url, QClipboard::Clipboard);

				const auto& text = QObject::tr ("Image pasted: %1, the URL was copied to the clipboard")
						.arg ("<em>" + url + "</em>");
				const auto& e = Util::MakeNotification (PLUGIN_VISIBLE_NAME, text, Priority::Info);
				GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			}
		}

		Util::Task<void> RunUpload (UploadContext ctx)
		{
			const auto servicePos = ctx.Services_.find (ctx.ServiceName_);
			if (servicePos == ctx.Services_.end ())
			{
				QMessageBox::critical (nullptr,
						PLUGIN_VISIBLE_NAME,
						QObject::tr ("Unknown upload service: %1.")
								.arg (ctx.ServiceName_));
				co_return;
			}

			const auto& service = servicePos.value ();

			auto row = ctx.ProgressManager_.AddRow ({
						.Name_ = QObject::tr ("Uploading image to %1").arg (ctx.ServiceName_),
						.Specific_ = ProcessInfo
						{
							.Parameters_ = FromUserInitiated,
							.Kind_ = ProcessKind::Upload,
						}
					},
					{ .Total_ = ctx.Data_.size () });

			const auto reply = Post (service, ctx.Data_, ctx.Fmt_, *GetProxyHolder ()->GetNetworkAccessManager ());
			QObject::connect (reply,
					&QNetworkReply::uploadProgress,
					[row = std::move (row)] (qint64 done, qint64 total)
					{
						row->SetDone (done);
						row->SetTotal (total);
					});

			const auto result = co_await *reply;
			if (const auto error = result.IsError ())
			{
				qWarning () << ctx.ServiceName_ << "failed with" << *error;
				const auto& text = QObject::tr ("Image upload failed: %1")
						.arg (error->ErrorText_);
				co_return NotifyError (ctx, text);
			}

			const auto& replyBody = result.GetReplyData ();
			const auto& url = co_await Util::WithHandler (service.GetLink_ (replyBody),
					[&] (auto&&)
					{
						qWarning () << ctx.ServiceName_ << "unable to get link from" << replyBody;
						const auto& text = QObject::tr ("Image upload to %1 failed: service error.")
								.arg ("<em>" + ctx.ServiceName_ + "</em>");
						NotifyError (ctx, text);
					});

			HandleSuccess (url, ctx);
		}

		void TryAnotherService (UploadContext ctx)
		{
			ctx.Services_.remove (ctx.ServiceName_);

			bool ok = false;
			ctx.ServiceName_ = QInputDialog::getItem (nullptr,
					PLUGIN_VISIBLE_NAME,
					QObject::tr ("Please select another service to try:"),
					ctx.Services_.keys (),
					0,
					false,
					&ok);
			if (ok)
				RunUpload (ctx);
		}
	}

	void Upload (const QByteArray& data, QSize dim, const Entity& e, Format fmt, Util::ProgressManager& progress)
	{
		auto callback = e.Additional_ ["DataFilterCallback"].value<DataFilterCallback_f> ();
		auto dataFilter = e.Additional_ ["DataFilter"].toString ();

		QHash<QString, HostingService> allServices;
		for (const auto& service : GetAllServices ())
			if (service.Accepts_ ({ .Size_ = static_cast<quint64> (data.size ()), .Dim_ = dim }))
				allServices [service.Name_] = service;
		RunUpload ({ data, fmt, progress, callback, allServices, dataFilter });
	}
}
