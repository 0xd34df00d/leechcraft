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
		template<quint64 SizeLimit>
		auto CheckSize (const ImageInfo& info)
		{
			return info.Size_ <= SizeLimit;
		}

		namespace Imagebin
		{
			QNetworkReply* Post (const QByteArray& data, Format fmt, QNetworkAccessManager *am)
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

			HostingService::Result_t GetLink (const QString& contents)
			{
				const auto& lines = contents.split ('\n');
				const auto pos = std::find_if (lines.begin (), lines.end (),
						[] (const QString& line) { return line.startsWith ("url:"); });

				if (pos == lines.end ())
				{
					qWarning () << Q_FUNC_INFO
							<< "no URL:"
							<< contents;
					return HostingService::Result_t::Left ({});
				}

				return HostingService::Result_t::Right (pos->section (':', 1));
			}
		}

		namespace Catbox
		{
			QNetworkReply* Post (const QByteArray& data, Format fmt, QNetworkAccessManager *am)
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

			HostingService::Result_t GetLink (const QString& contents)
			{
				return HostingService::Result_t::Right (contents);
			}
		}

		namespace PomfLike
		{
			auto Post (const QString& uploadUrl)
			{
				return [=] (const QByteArray& data, Format fmt, QNetworkAccessManager *am)
				{
					auto mp = BuildRequest ({}, { .Format_ = fmt, .FieldName_ = "files[]"_qba, .Data_ = data });
					auto reply = am->post (QNetworkRequest { uploadUrl }, &*mp);
					Util::ReleaseInto (std::move (mp), *reply);
					return reply;
				};
			}

			auto GetLink (const QString& prefix)
			{
				return [=] (const QString& body)
				{
					const auto& json = Util::ParseJson (body.toUtf8 (), "LC::Imgaste::PomfLikeService::GetLink()");
					if (json.isNull ())
						return HostingService::Result_t::Left ({});

					const auto filename = json.toMap () ["files"]
							.toList ().value (0)
							.toMap () ["url"].toString ();
					return HostingService::Result_t::Right (prefix + filename);
				};
			}
		}
	}

	Q_DECL_EXPORT const QVector<HostingService>& GetAllServices ()
	{
		static const QVector<HostingService> list
		{
			{ .Name_ = "imagebin.ca"_qs, .Accepts_ = CheckSize<15_mib>, .Post_ = Imagebin::Post, .GetLink_ = Imagebin::GetLink },
			{ .Name_ = "catbox.moe"_qs, .Accepts_ = CheckSize<75_mib>, .Post_ = Catbox::Post, .GetLink_ = Catbox::GetLink },
			{ .Name_ = "pomf.cat"_qs, .Accepts_ = CheckSize<200_mib>, .Post_ = PomfLike::Post ("https://pomf.cat/upload.php"), .GetLink_ = PomfLike::GetLink ("https://a.pomf.cat/") }
		};

		return list;
	}
}
