/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imgaste.h"
#include <QIcon>
#include <QBuffer>
#include <QUrl>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QImageReader>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <util/sys/mimedetector.h>
#include <util/threads/futures.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/entitytesthandleresult.h>
#include "hostingservice.h"
#include "poster.h"

namespace LC::Imgaste
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("imgaste");
		Proxy_ = proxy;

		ReprModel_ = new QStandardItemModel { this };
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Imgaste";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Imgaste";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Simple image uploader to imagebin services like pomf.cat or imagebin.ca.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (e.Mime_ != "x-leechcraft/data-filter-request")
			return {};

		const auto& image = e.Entity_.value<QImage> ();
		if (!image.isNull ())
			return EntityTestHandleResult { EntityTestHandleResult::PIdeal };

		const auto& localFile = e.Entity_.toUrl ().toLocalFile ();
		if (!QFile::exists (localFile))
			return {};

		if (Util::DetectFileMime (localFile).startsWith ("image/"))
			return EntityTestHandleResult { EntityTestHandleResult::PHigh };

		return {};
	}

	void Plugin::Handle (Entity e)
	{
		const auto& img = e.Entity_.value<QImage> ();
		const auto& localFile = e.Entity_.toUrl ().toLocalFile ();
		if (!img.isNull ())
			UploadImage (img, e);
		else if (QFile::exists (localFile))
			UploadFile (localFile, e);
		else
			qWarning () << Q_FUNC_INFO
					<< "unhandled entity"
					<< e.Entity_;
	}

	QString Plugin::GetFilterVerb () const
	{
		return tr ("Upload image");
	}

	namespace
	{
		std::optional<IDataFilter::FilterVariant> ToFilterVariant (HostingService s,
				const ImageInfo& imageInfo)
		{
			const auto& hostingInfo = ToInfo (s);
			if (!hostingInfo.Accepts_ (imageInfo))
				return {};

			const auto& str = hostingInfo.Name_;
			return { { str.toUtf8 (), str, {}, {} } };
		}

		std::optional<ImageInfo> GetImageInfo (const QVariant& data)
		{
			const auto& file = data.toUrl ().toLocalFile ();
			const QFileInfo fileInfo { file };
			if (fileInfo.exists ())
			{
				const quint64 filesize = fileInfo.size ();
				return { { filesize, QImageReader { file }.size () } };
			}
			else if (data.canConvert<QImage> ())
				return { { 0, data.value<QImage> ().size () } };
			else
				return {};
		}
	}

	QList<IDataFilter::FilterVariant> Plugin::GetFilterVariants (const QVariant& data) const
	{
		const auto& maybeInfo = GetImageInfo (data);
		if (!maybeInfo)
			return {};

		const auto& info = *maybeInfo;

		QList<IDataFilter::FilterVariant> result;
		for (const auto& item : GetAllServices ())
			if (const auto res = ToFilterVariant (item, info))
				result << *res;
		return result;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return ReprModel_;
	}

	void Plugin::UploadFile (const QString& name, const Entity& e)
	{
		QFile file { name };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file:"
					<< file.errorString ();
			return;
		}

		const auto& format = QString::fromLatin1 (Util::DetectFileMime (name)).section ('/', 1, 1);

		UploadImpl (file.readAll (), e, format);
	}

	void Plugin::UploadImage (const QImage& img, const Entity& e)
	{
		const auto& format = e.Additional_.value ("Format", "PNG").toString ();

		QByteArray bytes;
		QBuffer buf (&bytes);
		buf.open (QIODevice::ReadWrite);
		if (!img.save (&buf,
					qPrintable (format),
					e.Additional_ ["Quality"].toInt ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "save failed";
			return;
		}

		UploadImpl (buf.data (), e, format);
	}

	void Plugin::UploadImpl (const QByteArray& data, const Entity& e, const QString& format)
	{
		const auto& dataFilter = e.Additional_ ["DataFilter"].toString ();
		const auto& type = FromString (dataFilter);
		if (!type)
		{
			QMessageBox::critical (nullptr,
					"LeechCraft",
					tr ("Unknown upload service: %1.")
						.arg (dataFilter));
			return;
		}

		const auto& callback = e.Additional_ ["DataFilterCallback"].value<DataFilterCallback_f> ();

		const auto em = Proxy_->GetEntityManager ();

		auto poster = new Poster (*type,
				data,
				format,
				Proxy_,
				ReprModel_);
		Util::Sequence (this, poster->GetFuture ()) >>
				Util::Visitor
				{
					[callback, em] (const QString& url)
					{
						if (!callback)
						{
							QApplication::clipboard ()->setText (url, QClipboard::Clipboard);

							auto text = tr ("Image pasted: %1, the URL was copied to the clipboard")
									.arg ("<em>" + url + "</em>");
							em->HandleEntity (Util::MakeNotification ("Imgaste", text, Priority::Info));
						}
						else
							callback (url);
					},
					Util::Visitor
					{
						[em] (const Poster::NetworkRequestError& error)
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
						},
						[em, dataFilter] (const Poster::ServiceAPIError&)
						{
							qWarning () << Q_FUNC_INFO
									<< dataFilter;

							const auto& text = tr ("Image upload to %1 failed: service error.")
									.arg ("<em>" + dataFilter + "</em>");
							em->HandleEntity (Util::MakeNotification ("Imgaste", text, Priority::Critical));
						}
					}
				};
	}
}

LC_EXPORT_PLUGIN (leechcraft_imgaste, LC::Imgaste::Plugin);
