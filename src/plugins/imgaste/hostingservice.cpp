/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hostingservice.h"
#include <algorithm>
#include <QtDebug>
#include <QNetworkReply>
#include <QUrl>
#include <QStringList>
#include <QHttpMultiPart>
#include <util/sll/either.h>
#include <util/sll/json.h>
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
			auto Upload (const QByteArray& data, Format fmt)
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

			HostingService::Result_t GetLink (QStringView contents)
			{
				constexpr static QStringView urlMarker = u"url:"_qsv;

				const auto& lines = contents.split ('\n');
				const auto pos = std::ranges::find_if (lines, [] (auto line) { return line.startsWith (urlMarker); });
				if (pos == lines.end ())
					return { Util::AsLeft, {} };
				return pos->mid (urlMarker.size ()).toString ();
			}
		}

		namespace Catbox
		{
			auto Upload (const QByteArray& data, Format fmt)
			{
				return BuildRequest ({
							{ "reqtype"_qba, "fileupload"_qba },
							{ "userhash"_qba, {} },
						},
						{ .Format_ = fmt, .FieldName_ = "fileToUpload"_qba, .Data_ = data });
			}
		}

		HostingService::Result_t GetLinkTrimmed (QString body)
		{
			body = std::move (body).trimmed ();
			if (QUrl { body }.isValid ())
				return body;

			return { Util::AsLeft, {} };
		}

		auto SimpleUpload (const QByteArray& fieldName)
		{
			return [=] (const QByteArray& data, Format fmt)
			{
				return BuildRequest ({ fmt, fieldName, data });
			};
		}

		namespace Tinystash
		{
			auto Upload (const QByteArray& data, Format fmt, QNetworkAccessManager& am)
			{
				QNetworkRequest req { QUrl { "https://tinystash.undef.im/upload/file"_qs } };
				req.setRawHeader ("Origin", "https://tinystash.undef.im");
				req.setRawHeader ("Referer", "https://tinystash.undef.im/");

				req.setRawHeader ("app-id", "arbitrary string");
				req.setHeader (QNetworkRequest::ContentTypeHeader, GetMimeType (fmt));
				req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());
				return am.post (req, data);
			}
		}

		namespace PomfLike
		{
			[[maybe_unused]]
			HostingService::Result_t GetLink (const QString& body)
			{
				try
				{
					using Util::As;
					using enum QJsonValue::Type;

					const auto& json = As<Object> (Util::ToJson (body.toUtf8 ()).GetRight ());
					const auto& files = As<Array> (json [u"files"_qsv]);
					const auto& file = As<Object> (files [0]);
					const auto& filename = As<String> (file [u"url"_qsv]);
					return filename;
				}
				catch (const std::exception&)
				{
					return { Util::AsLeft, {} };
				}
			}
		}
	}

	Q_DECL_EXPORT const QVector<HostingService>& GetAllServices ()
	{
		using namespace std::chrono_literals;

		static const QVector<HostingService> list
		{
			{
				.Name_ = "0x0.st"_qs,
				.UploadUrl_ { "https://0x0.st/"_qs },
				.Accepts_ = CheckSize<512_mib>,
				.Upload_ = SimpleUpload ("file"),
				.GetLink_ = GetLinkTrimmed
			},
			{
				.Name_ = "catbox.moe"_qs,
				.UploadUrl_ { "https://catbox.moe/user/api.php"_qs },
				.Accepts_ = CheckSize<75_mib>,
				.Upload_ = Catbox::Upload,
				.GetLink_ = GetLinkTrimmed
			},
			{
				.Name_ = "uguu.se",
				.UploadUrl_ { "https://uguu.se/upload?output=text"_qs },
				.Expiration_ = { 3h },
				.Accepts_ = CheckSize<64_mib>,
				.Upload_ = SimpleUpload ("files[]"),
				.GetLink_ = GetLinkTrimmed
			},
			{
				.Name_ = "tinystash.undef.im",
				.UploadUrl_ { "https://tinystash.undef.im/upload/file"_qs },
				.Accepts_ = CheckSize<20_mib>,
				.Upload_ = Tinystash::Upload,
				.GetLink_ = GetLinkTrimmed
			},
			{
				.Name_ = "imagebin.ca"_qs,
				.UploadUrl_ { "https://imagebin.ca/upload.php"_qs },
				.Accepts_ = CheckSize<15_mib>,
				.Upload_ = Imagebin::Upload,
				.GetLink_ = Imagebin::GetLink
			},
		};

		return list;
	}

	Q_DECL_EXPORT QNetworkReply* Post (const HostingService& service, const QByteArray& data, Format fmt, QNetworkAccessManager& am)
	{
		return Util::Visit (service.Upload_,
				[&] (const ReplyUploader& uploader) { return uploader (data, fmt, am); },
				[&] (const MultipartUploader& uploader)
				{
					const auto& origin = service.UploadUrl_.toEncoded (QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment);
					auto mp = uploader (data, fmt);

					QNetworkRequest request { service.UploadUrl_ };
					request.setRawHeader ("Origin", origin);
					request.setRawHeader ("Referer", origin + '/');
					auto reply = am.post (request, &*mp);
					Util::ReleaseInto (std::move (mp), *reply);
					return reply;
				});
	}
}
