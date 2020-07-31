/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hostingservice.h"
#include <QtDebug>
#include <QNetworkReply>
#include <QUrl>
#include <QRegExp>
#include <QStringList>
#include <util/sll/either.h>
#include <util/sll/parsejson.h>
#include <util/sll/unreachable.h>
#include "requestbuilder.h"

namespace LC::Imgaste
{
	bool operator< (HostingService s1, HostingService s2)
	{
		return static_cast<int> (s1) < static_cast<int> (s2);
	}

	auto MakeChecker (quint64 sizeLimit, const QSize& dimLimit)
	{
		return [=] (const ImageInfo& info)
		{
			return info.Size_ <= sizeLimit &&
					info.Dim_.width () <= dimLimit.width () &&
					info.Dim_.height () <= dimLimit.height ();
		};
	}

	auto MakeChecker (quint64 sizeLimit)
	{
		return [=] (const ImageInfo& info) { return info.Size_ <= sizeLimit; };
	}

	constexpr quint64 operator"" _mib (quint64 mibs)
	{
		return mibs * 1024 * 1024;
	}

	HostingServiceInfo ToInfo (HostingService s)
	{
		switch (s)
		{
		case HostingService::ImagebinCa:
			return { "imagebin.ca", MakeChecker (15_mib) };
		case HostingService::PomfCat:
			return { "pomf.cat", MakeChecker (75_mib) };
		case HostingService::CatboxMoe:
			return { "catbox.moe", MakeChecker (200_mib) };
		}

		Util::Unreachable ();
	}

	std::optional<HostingService> FromString (const QString& str)
	{
		for (auto s : GetAllServices ())
			if (ToInfo (s).Name_ == str)
				return s;

		qWarning () << Q_FUNC_INFO
				<< "unknown hosting service"
				<< str;
		return {};
	}

	QList<HostingService> GetAllServices ()
	{
		return
		{
			HostingService::PomfCat,
			HostingService::ImagebinCa,
			HostingService::CatboxMoe,
		};
	}

	namespace
	{
		QNetworkRequest PrefillRequest (const QUrl& url, const RequestBuilder& builder)
		{
			QNetworkRequest request { url };
			request.setHeader (QNetworkRequest::ContentTypeHeader,
					QString ("multipart/form-data; boundary=" + builder.GetBoundary ()));
			request.setHeader (QNetworkRequest::ContentLengthHeader, QString::number (builder.Size ()));
			return request;
		}

		struct ImagebinWorker final : Worker
		{
			QNetworkReply* Post (const QByteArray& data, const QString& fmt, QNetworkAccessManager *am) const override
			{
				QUrl url { "https://imagebin.ca/upload.php" };

				RequestBuilder builder;
				builder.AddPair ("t", "file");

				builder.AddPair ("title", "");
				builder.AddPair ("description", "");
				builder.AddPair ("tags", "leechcraft");
				builder.AddPair ("category", "general");
				builder.AddPair ("private", "true");
				builder.AddFile (fmt, "file", data);

				QByteArray formed = builder.Build ();

				auto request = PrefillRequest (url, builder);
				request.setRawHeader ("Origin", "https://imagebin.ca");
				request.setRawHeader ("Referer", "https://imagebin.ca/");
				return am->post (request, formed);
			}

			Result_t GetLink (const QString& contents, const Headers_t&) const override
			{
				const auto& lines = contents.split ('\n');
				const auto pos = std::find_if (lines.begin (), lines.end (),
						[] (const QString& line) { return line.startsWith ("url:"); });

				if (pos == lines.end ())
				{
					qWarning () << Q_FUNC_INFO
							<< "no URL:"
							<< contents;
					return Result_t::Left ({});
				}

				return Result_t::Right (pos->section (':', 1));
			}
		};

		struct CatboxWorker final : Worker
		{
			QNetworkReply* Post (const QByteArray& data, const QString& fmt, QNetworkAccessManager *am) const override
			{
				QUrl url { "https://catbox.moe/user/api.php" };

				RequestBuilder builder;
				builder.AddPair ("reqtype", "fileupload");
				builder.AddPair ("userhash", {});
				builder.AddFile (fmt, "fileToUpload", data);

				auto formed = builder.Build ();

				auto request = PrefillRequest (url, builder);
				request.setRawHeader ("Origin", "https://catbox.moe");
				request.setRawHeader ("Referer", "https://catbox.moe/");
				return am->post (request, formed);
			}

			Result_t GetLink (const QString& contents, const Headers_t&) const override
			{
				qDebug () << Q_FUNC_INFO << contents;
				return Result_t::Right (contents);
			}
		};

		struct PomfLikeWorker final : Worker
		{
			const QString Prefix_;
			const QUrl UploadUrl_;

			PomfLikeWorker (const QString& prefix, const QUrl& uploadUrl)
			: Prefix_ { prefix }
			, UploadUrl_ { uploadUrl }
			{
			}

			QNetworkReply* Post (const QByteArray& data, const QString& fmt, QNetworkAccessManager *am) const override
			{
				RequestBuilder builder;
				builder.AddFile (fmt, "files[]", data);

				return am->post (PrefillRequest (UploadUrl_, builder), builder.Build ());
			}

			Result_t GetLink (const QString& body, const Headers_t&) const override
			{
				const auto& json = Util::ParseJson (body.toUtf8 (), Q_FUNC_INFO);
				const auto filename = json.toMap () ["files"]
						.toList ().value (0)
						.toMap () ["url"].toString ();
				return Result_t::Right (Prefix_ + filename);
			}
		};
	}

	Worker_ptr MakeWorker (HostingService s)
	{
		switch (s)
		{
		case HostingService::ImagebinCa:
			return std::make_unique<ImagebinWorker> ();
		case HostingService::PomfCat:
			return std::make_unique<PomfLikeWorker> ("https://a.pomf.cat/",
					QUrl { "https://pomf.cat/upload.php" });
		case HostingService::CatboxMoe:
			return std::make_unique<CatboxWorker> ();
		}

		Util::Unreachable ();
	}
}
