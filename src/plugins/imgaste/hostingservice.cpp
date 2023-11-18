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
#include <QHttpMultiPart>
#include <util/sll/either.h>
#include <util/sll/parsejson.h>
#include <util/sll/qtutil.h>
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

			QNetworkReply* Post (const QByteArray& data, Format fmt, QNetworkAccessManager *am) const override
			{
				const QUrl url { "https://imagebin.ca/upload.php"_qs };

				auto mp = BuildRequest ({
							{ "t"_qba, "file"_qba },
							{ "title"_qba, ""_qba },
							{ "description"_qba, ""_qba },
							{ "tags"_qba, "leechcraft"_qba },
							{ "category"_qba, "general"_qba },
							{ "private"_qba, "true"_qba },
						},
						{ .Format_ = fmt, .FieldName_ = "file"_qba, .Data_ = data });

				QNetworkRequest request { url };
				request.setRawHeader ("Origin", "https://imagebin.ca");
				request.setRawHeader ("Referer", "https://imagebin.ca/");
				auto reply = am->post (request, &*mp);
				Util::ReleaseInto (std::move (mp), *reply);
				return reply;
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

			QNetworkReply* Post (const QByteArray& data, Format fmt, QNetworkAccessManager *am) const override
			{
				QUrl url { "https://catbox.moe/user/api.php" };

				auto mp = BuildRequest ({
						{ "reqtype"_qba, "fileupload"_qba },
						{ "userhash"_qba, {} },
					}, { .Format_ = fmt, .FieldName_ = "fileToUpload"_qba, .Data_ = data });

				QNetworkRequest request { url };
				request.setRawHeader ("Origin", "https://catbox.moe");
				request.setRawHeader ("Referer", "https://catbox.moe/");
				auto reply = am->post (request, &*mp);
				Util::ReleaseInto (std::move (mp), *reply);
				return reply;
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

			QNetworkReply* Post (const QByteArray& data, Format fmt, QNetworkAccessManager *am) const override
			{
				auto mp = BuildRequest ({}, { .Format_ = fmt, .FieldName_ = "files[]"_qba, .Data_ = data });
				auto reply = am->post (QNetworkRequest { UploadUrl_ }, &*mp);
				Util::ReleaseInto (std::move (mp), *reply);
				return reply;
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
