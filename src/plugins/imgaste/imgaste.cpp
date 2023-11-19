/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imgaste.h"
#include <optional>
#include <QIcon>
#include <QBuffer>
#include <QUrl>
#include <QStandardItemModel>
#include <QImageReader>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <util/sll/qtutil.h>
#include <util/sys/mimedetector.h>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include "hostingservice.h"
#include "uploader.h"

namespace LC::Imgaste
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("imgaste"_qs);

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
		return PLUGIN_VISIBLE_NAME;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Simple image uploader to imagebin services like pomf.cat or imagebin.ca.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
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
		std::optional<IDataFilter::FilterVariant> ToFilterVariant (const HostingService& service,
				const ImageInfo& imageInfo)
		{
			if (!service.Accepts (imageInfo))
				return {};

			const auto& str = service.GetName ();
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
			if (const auto res = ToFilterVariant (*item, info))
				result << *res;
		return result;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return ReprModel_;
	}

	namespace
	{
		Format GuessFormat (const QString& format)
		{
			if (!format.compare ("png", Qt::CaseInsensitive))
				return Format::PNG;
			if (!format.compare ("jpg", Qt::CaseInsensitive))
				return Format::JPG;

			qWarning () << "unknown format:" << format;
			return Format::PNG;
		}
	}

	void Plugin::UploadFile (const QString& name, const Entity& e)
	{
		QFile file { name };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file:"
					<< file.errorString ();
			QMessageBox::critical (nullptr,
					PLUGIN_VISIBLE_NAME,
					tr ("Unable to open file: %1.")
							.arg (file.errorString ()));
			return;
		}

		const auto& formatStr = QString::fromLatin1 (Util::DetectFileMime (name)).section ('/', 1, 1);

		UploadImpl (file.readAll (), e, GuessFormat (formatStr));
	}

	void Plugin::UploadImage (const QImage& img, const Entity& e)
	{
		const auto& formatStr = e.Additional_.value ("Format", "PNG").toString ();

		QByteArray bytes;
		QBuffer buf (&bytes);
		buf.open (QIODevice::ReadWrite);
		if (!img.save (&buf,
					qPrintable (formatStr),
					e.Additional_ ["Quality"].toInt ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "save failed";
			QMessageBox::critical (nullptr,
					PLUGIN_VISIBLE_NAME,
					tr ("Unable to serialize image."));
			return;
		}

		UploadImpl (buf.data (), e, GuessFormat (formatStr));
	}

	void Plugin::UploadImpl (const QByteArray& data, const Entity& e, Format format)
	{
		const auto& callback = e.Additional_ ["DataFilterCallback"].value<DataFilterCallback_f> ();

		const auto uploader = new Uploader { data, format, callback, ReprModel_ };
		uploader->Upload (e.Additional_ ["DataFilter"].toString ());
	}
}

LC_EXPORT_PLUGIN (leechcraft_imgaste, LC::Imgaste::Plugin);
