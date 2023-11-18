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
#include <QStringList>
#include <util/sll/either.h>
#include <util/sll/parsejson.h>
#include <util/sll/udls.h>
#include "requestbuilder.h"

namespace LC::Imgaste
{
	namespace
	{
		bool CheckImage (const ImageInfo& info, quint64 sizeLimit)
		{
			return info.Size_ <= sizeLimit;
		}

		QNetworkRequest PrefillRequest (const QUrl& url, const RequestBuilder& builder)
		{
			QNetworkRequest request { url };
			request.setHeader (QNetworkRequest::ContentTypeHeader,
					QString ("multipart/form-data; boundary=" + builder.GetBoundary ()));
			request.setHeader (QNetworkRequest::ContentLengthHeader, QString::number (builder.Size ()));
			return request;
		}

		struct ImagebinService final : HostingService
		{
			QString GetName () const override
			{
				return "imagebin.ca"_qs;
			}

			bool Accepts (const ImageInfo& info) const override
			{
				return CheckImage (info, 15_mib);
			}

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

		struct CatboxService final : HostingService
		{
			QString GetName () const override
			{
				return "catbox.moe";
			}

			bool Accepts (const ImageInfo& info) const override
			{
				return CheckImage (info, 75_mib);
			}

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

		class PomfLikeService final : public HostingService
		{
			QString Name_;
			QString Prefix_;
			QUrl UploadUrl_;
		public:
			PomfLikeService (QString name, QString prefix, QUrl uploadUrl)
			: Name_ { std::move (name) }
			, Prefix_ { std::move (prefix) }
			, UploadUrl_ { std::move (uploadUrl) }
			{
			}

			QString GetName () const override
			{
				return Name_;
			}

			bool Accepts (const ImageInfo& info) const override
			{
				return CheckImage (info, 200_mib);
			}

			QNetworkReply* Post (const QByteArray& data, const QString& fmt, QNetworkAccessManager *am) const override
			{
				RequestBuilder builder;
				builder.AddFile (fmt, "files[]", data);

				return am->post (PrefillRequest (UploadUrl_, builder), builder.Build ());
			}

			Result_t GetLink (const QString& body, const Headers_t&) const override
			{
				const auto& json = Util::ParseJson (body.toUtf8 (), "LC::Imgaste::PomfLikeService::GetLink()");
				if (json.isNull ())
					return Result_t::Left ({});

				const auto filename = json.toMap () ["files"]
						.toList ().value (0)
						.toMap () ["url"].toString ();
				return Result_t::Right (Prefix_ + filename);
			}
		};
	}

	const QList<std::shared_ptr<HostingService>>& GetAllServices ()
	{
		static const QList<std::shared_ptr<HostingService>> list
		{
			std::make_shared<ImagebinService> (),
			std::make_shared<CatboxService> (),
			std::make_shared<PomfLikeService> ("pomf.cat", "https://a.pomf.cat/", QUrl { "https://pomf.cat/upload.php" }),
		};

		return list;
	}
}
