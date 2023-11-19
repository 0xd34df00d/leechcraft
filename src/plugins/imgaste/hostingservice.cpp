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
		auto CheckSize (ImageInfo info)
		{
			return info.Size_ <= SizeLimit;
		}

		namespace Imagebin
		{
			auto MakeMultiPart (const QByteArray& data, Format fmt)
			{
				return BuildRequest ({
							{ "t"_qba, "file"_qba },
							{ "title"_qba, ""_qba },
							{ "description"_qba, ""_qba },
							{ "tags"_qba, "leechcraft"_qba },
							{ "category"_qba, "general"_qba },
							{ "private"_qba, "true"_qba },
						},
						{ .Format_ = fmt, .FieldName_ = "file"_qba, .Data_ = data });
			}

			HostingService::Result_t GetLink (const QString& contents)
			{
				constexpr static QStringView urlMarker = u"url:"_qsv;

				const auto& lines = contents.splitRef ('\n');
				const auto pos = std::find_if (lines.begin (), lines.end (),
						[] (const QStringRef& line) { return line.startsWith (urlMarker); });

				if (pos == lines.end ())
				{
					qWarning () << "no URL:"
							<< contents;
					return HostingService::Result_t::Left ({});
				}

				return HostingService::Result_t::Right (pos->mid (urlMarker.size ()).toString ());
			}
		}

		namespace Catbox
		{
			auto MakeMultiPart (const QByteArray& data, Format fmt)
			{
				return BuildRequest ({
							{ "reqtype"_qba, "fileupload"_qba },
							{ "userhash"_qba, {} },
						},
						{ .Format_ = fmt, .FieldName_ = "fileToUpload"_qba, .Data_ = data });
			}

			HostingService::Result_t GetLink (const QString& contents)
			{
				return HostingService::Result_t::Right (contents);
			}
		}

		namespace PomfLike
		{
			auto MakeMultiPart (const QByteArray& data, Format fmt)
			{
				return BuildRequest ({}, { .Format_ = fmt, .FieldName_ = "files[]"_qba, .Data_ = data });
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
			{ .Name_ = "imagebin.ca"_qs, .UploadUrl_ { "https://imagebin.ca/upload.php"_qs }, .Accepts_ = CheckSize<15_mib>, .MakeMultiPart_ = Imagebin::MakeMultiPart, .GetLink_ = Imagebin::GetLink },
			{ .Name_ = "catbox.moe"_qs, .UploadUrl_ { "https://catbox.moe/user/api.php"_qs }, .Accepts_ = CheckSize<75_mib>, .MakeMultiPart_ = Catbox::MakeMultiPart, .GetLink_ = Catbox::GetLink },
		};

		return list;
	}

	Q_DECL_EXPORT QNetworkReply* Post (const HostingService& service, const QByteArray& data, Format fmt, QNetworkAccessManager& am)
	{
		const auto& origin = service.UploadUrl_.toEncoded (QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
		auto mp = service.MakeMultiPart_ (data, fmt);

		QNetworkRequest request { service.UploadUrl_ };
		request.setRawHeader ("Origin", origin);
		request.setRawHeader ("Referer", origin + '/');
		auto reply = am.post (request, &*mp);
		Util::ReleaseInto (std::move (mp), *reply);
		return reply;
	}
}
