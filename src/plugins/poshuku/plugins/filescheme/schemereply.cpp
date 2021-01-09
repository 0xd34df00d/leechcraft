/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "schemereply.h"
#include <QTimer>
#include <QDir>
#include <QIcon>
#include <QSize>
#include <QPixmap>
#include <QFileIconProvider>
#include <QDateTime>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QtDebug>
#include <util/util.h>

namespace LC
{
namespace Poshuku
{
namespace FileScheme
{
	SchemeReply::SchemeReply (const QNetworkRequest& req, QObject *parent)
	: QNetworkReply (parent)
	{
		setOperation (QNetworkAccessManager::GetOperation);
		setRequest (req);
		setUrl (req.url ());

		Buffer_.open (QIODevice::ReadWrite);
		setError (NoError, tr ("No error"));

		QTimer::singleShot (0, this, &SchemeReply::List);
		open (QIODevice::ReadOnly);
	}

	SchemeReply::~SchemeReply ()
	{
		close ();
	}

	qint64 SchemeReply::bytesAvailable () const
	{
		return Buffer_.bytesAvailable () + QNetworkReply::bytesAvailable ();
	}

	void SchemeReply::abort ()
	{
	}

	void SchemeReply::close ()
	{
		Buffer_.close ();
	}

	qint64 SchemeReply::readData (char *data, qint64 maxSize)
	{
		return Buffer_.read (data, maxSize);
	}

	const int IconSize = 18;

	namespace
	{
		QString GetBase64 (const QIcon& icon, int size = IconSize)
		{
			QPixmap pix = icon.pixmap (QSize (size, size));
			QBuffer buf;
			buf.open (QIODevice::ReadWrite);
			if (!pix.save (&buf, "PNG"))
			{
				pix = QPixmap (size, size);
				pix.fill (Qt::transparent);
				buf.buffer ().clear ();
				pix.save (&buf, "PNG");
			}
			return buf.buffer ().toBase64 ();
		}
	};

	void SchemeReply::List ()
	{
		QDir dir (url ().toLocalFile ());
		if (!dir.exists ())
		{
			setError (ContentNotFoundError,
					tr ("%1: no such file or directory")
						.arg (dir.absolutePath ()));
			emit error (ContentNotFoundError);
			emit finished ();
			return;
		}

		if (!dir.isReadable ())
		{
			setError (ContentAccessDenied,
					tr ("Unable to read %1")
						.arg (dir.absolutePath ()));
			emit error (ContentAccessDenied);
			emit finished ();
			return;
		}

		QFile basetplFile (":/plugins/poshuku/plugins/filescheme/basetemplate.tpl");
		if (!basetplFile.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to open"
				<< basetplFile.fileName ();
			return;
		}

		QFile rowtplFile (":/plugins/poshuku/plugins/filescheme/rowtemplate.tpl");
		if (!rowtplFile.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to open"
				<< rowtplFile.fileName ();
			return;
		}

		QString html = basetplFile.readAll ();
		QString row = rowtplFile.readAll ();
		QString link = "<a href=\"%1\">%2</a>";

		QString rows;

		if (!dir.isRoot ())
		{
			QIcon icon = qApp->style ()->standardIcon (QStyle::SP_FileDialogToParent);
			QString addr = QString::fromUtf8 (
						QUrl::fromLocalFile (
							QFileInfo (
								dir.absoluteFilePath ("..")
								).canonicalFilePath ()
							).toEncoded( )
						);
			rows += row
				.arg (GetBase64 (icon))
				.arg (link
						.arg (addr)
						.arg (".."))
				.arg (QString ())
				.arg (QString ());
		}

		QFileIconProvider iconProvider;

		const auto& list = dir.entryInfoList (QDir::AllEntries | QDir::Hidden,
				QDir::Name | QDir::DirsFirst);

		for (const auto& entry : list)
		{
			if (entry.fileName () == "." ||
					entry.fileName () == "..")
				continue;

			QUrl urlAddr = QUrl::fromLocalFile (entry.canonicalFilePath ());
			QString addr = QString::fromUtf8 (urlAddr.toEncoded ());
			QString size;
			if (entry.isFile ())
				size = Util::MakePrettySize (entry.size ());
			auto modified = QLocale {}.toString (entry.lastModified (), QLocale::ShortFormat);

			rows += row
				.arg (GetBase64 (iconProvider.icon (entry)))
				.arg (link
						.arg (addr)
						.arg (entry.fileName ()))
				.arg (size)
				.arg (modified);
		}

		html = html
			.arg (dir.absolutePath ())
			.arg (tr ("Contents of %1")
					.arg (dir.absolutePath ()))
			.arg (tr ("File"))
			.arg (tr ("Size"))
			.arg (tr ("Modified"))
			.arg (rows);

		QTextStream stream (&Buffer_);
		stream << html;
		stream.flush ();
		Buffer_.reset ();

		setHeader (QNetworkRequest::ContentTypeHeader,
				QByteArray ("text/html"));
		setHeader (QNetworkRequest::ContentLengthHeader,
				Buffer_.bytesAvailable ());
		setAttribute (QNetworkRequest::HttpStatusCodeAttribute,
				200);
		setAttribute (QNetworkRequest::HttpReasonPhraseAttribute,
				QByteArray ("OK"));
		emit metaDataChanged ();
		emit downloadProgress (Buffer_.size (), Buffer_.size ());
		NetworkError errorCode = error ();
		if (errorCode != NoError)
			emit error (errorCode);
		else if (Buffer_.size () > 0)
			emit readyRead ();

		emit finished ();
	}
}
}
}
