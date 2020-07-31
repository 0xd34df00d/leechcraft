/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "task.h"
#include <algorithm>
#include <stdexcept>
#include <QUrl>
#include <QFileInfo>
#include <QDataStream>
#include <QDir>
#include <QTimer>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/sll/qstringwrappers.h>
#include <util/sll/overload.h>
#include <util/threads/futures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace CSTP
{
	namespace
	{
		void LateDelete (QNetworkReply *rep)
		{
			if (rep)
			{
				Core::Instance ().RemoveFinishedReply (rep);
				rep->deleteLater ();
			}
		}

		QVariantMap Augment (QVariantMap map, const QList<QPair<QString, QVariant>>& pairs)
		{
			if (pairs.isEmpty ())
				return map;

			const auto& keys = Util::Map (Util::Stlize (map),
					[] (const std::pair<QString, QVariant>& p) { return std::move (p.first).trimmed (); });
			for (const auto& pair : pairs)
				if (!keys.contains (pair.first, Qt::CaseInsensitive))
					map [pair.first] = pair.second;

			return map;
		}
	}

	Task::Task (const QUrl& url, const QVariantMap& params)
	: Reply_ (nullptr, &LateDelete)
	, URL_ (url)
	, Timer_ (new QTimer (this))
	, Referer_ (params ["Referer"].toUrl ())
	, Operation_ (static_cast<QNetworkAccessManager::Operation> (params
				.value ("Operation", QNetworkAccessManager::GetOperation).toInt ()))
	, Headers_ (Augment (params.value ("HttpHeaders").toMap (),
			{
				{ "Content-Type", "application/x-www-form-urlencoded" }
			}))
	, UploadData_ (params.value ("UploadData").toByteArray ())
	{
		RestartTime ();

		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (updateInterface ()));
	}

	Task::Task (QNetworkReply *reply)
	: Reply_ (reply, &LateDelete)
	, Timer_ (new QTimer (this))
	, Operation_ (reply->operation ())
	, Headers_
	{
		{
			"Content-Type",
			reply->request ().header (QNetworkRequest::ContentTypeHeader).toByteArray ()
		}
	}
	{
		RestartTime ();

		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (updateInterface ()));

		Promise_.reportStarted ();
	}

	namespace
	{
		auto MapError (QNetworkReply::NetworkError error)
		{
			switch (error)
			{
			case QNetworkReply::NoError:
				return IDownload::Error::Type::NoError;

			case QNetworkReply::ConnectionRefusedError:
			case QNetworkReply::RemoteHostClosedError:
			case QNetworkReply::TimeoutError:
			case QNetworkReply::OperationCanceledError:
			case QNetworkReply::TemporaryNetworkFailureError:
			case QNetworkReply::NetworkSessionFailedError:
			case QNetworkReply::UnknownNetworkError:
				return IDownload::Error::Type::NetworkError;

			case QNetworkReply::SslHandshakeFailedError:
			case QNetworkReply::BackgroundRequestNotAllowedError:
			case QNetworkReply::TooManyRedirectsError:
			case QNetworkReply::InsecureRedirectError:
			case QNetworkReply::ProtocolUnknownError:
			case QNetworkReply::ProtocolInvalidOperationError:
			case QNetworkReply::ProtocolFailure:
				return IDownload::Error::Type::ProtocolError;

			case QNetworkReply::HostNotFoundError:
			case QNetworkReply::ContentNotFoundError:
				return IDownload::Error::Type::NotFound;

			case QNetworkReply::ContentGoneError:
				return IDownload::Error::Type::Gone;

			case QNetworkReply::ContentAccessDenied:
				return IDownload::Error::Type::AccessDenied;

			case QNetworkReply::AuthenticationRequiredError:
				return IDownload::Error::Type::AuthRequired;

			case QNetworkReply::ContentOperationNotPermittedError:
			case QNetworkReply::ContentReSendError:
			case QNetworkReply::ContentConflictError:
			case QNetworkReply::UnknownContentError:
				return IDownload::Error::Type::ContentError;

			case QNetworkReply::ProxyConnectionRefusedError:
			case QNetworkReply::ProxyConnectionClosedError:
			case QNetworkReply::ProxyNotFoundError:
			case QNetworkReply::ProxyTimeoutError:
			case QNetworkReply::ProxyAuthenticationRequiredError:
			case QNetworkReply::UnknownProxyError:
				return IDownload::Error::Type::ProxyError;

			case QNetworkReply::InternalServerError:
			case QNetworkReply::OperationNotImplementedError:
			case QNetworkReply::ServiceUnavailableError:
			case QNetworkReply::UnknownServerError:
				return IDownload::Error::Type::ServerError;
			}

			return IDownload::Error::Type::Unknown;
		}
	}

	void Task::Start (const std::shared_ptr<QFile>& tof)
	{
		FileSizeAtStart_ = tof->size ();
		To_ = tof;

		if (!Reply_)
		{
			if (URL_.scheme () == "file")
			{
				QTimer::singleShot (100,
						this,
						SLOT (handleLocalTransfer ()));
				return;
			}

			auto ua = XmlSettingsManager::Instance ().property ("UserUserAgent").toString ();
			if (ua.isEmpty ())
				ua = XmlSettingsManager::Instance ().property ("PredefinedUserAgent").toString ();

			if (ua == "%leechcraft%")
				ua = "LeechCraft.CSTP/" + Core::Instance ().GetCoreProxy ()->GetVersion ();

			QNetworkRequest req { URL_ };
			if (tof->size ())
				req.setRawHeader ("Range", QString ("bytes=%1-").arg (tof->size ()).toLatin1 ());
			req.setRawHeader ("User-Agent", ua.toLatin1 ());

			if (Referer_.isEmpty ())
				req.setRawHeader ("Referer", QString (QString ("http://") + URL_.host ()).toLatin1 ());
			else
				req.setRawHeader ("Referer", Referer_.toEncoded ());

			req.setRawHeader ("Host", URL_.host ().toLatin1 ());
			req.setRawHeader ("Origin", URL_.scheme ().toLatin1 () + "://" + URL_.host ().toLatin1 ());
			req.setRawHeader ("Accept", "*/*");

			RestartTime ();

			for (const auto& pair : Util::Stlize (Headers_))
				req.setRawHeader (pair.first.toLatin1 (), pair.second.toByteArray ());

			auto nam = Core::Instance ().GetNetworkAccessManager ();
			switch (Operation_)
			{
			case QNetworkAccessManager::GetOperation:
				Reply_.reset (nam->get (req));
				break;
			case QNetworkAccessManager::PostOperation:
				Reply_.reset (nam->post (req, UploadData_));
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unsupported operation";
				HandleError (IDownload::Error::Type::ProtocolError, tr ("Unsupported operation."));
				return;
			}
		}
		else
		{
			handleMetaDataChanged ();

			qint64 contentLength = Reply_->header (QNetworkRequest::ContentLengthHeader).toInt ();
			if (contentLength &&
					Reply_->bytesAvailable () == contentLength)
			{
				handleReadyRead ();
				handleFinished ();
				return;
			}
			else if (!Reply_->isOpen ())
			{
				qWarning () << Q_FUNC_INFO
						<< "reply is not open";
				HandleError (IDownload::Error::Type::LocalError, "Reply is not open.");
				return;
			}
			else if (handleReadyRead ())
				return;
		}

		if (!Timer_->isActive ())
			Timer_->start (3000);

		Reply_->setParent (nullptr);
		connect (Reply_.get (),
				SIGNAL (downloadProgress (qint64, qint64)),
				this,
				SLOT (handleDataTransferProgress (qint64, qint64)));
		connect (Reply_.get (),
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (Reply_.get (),
				SIGNAL (metaDataChanged ()),
				this,
				SLOT (handleMetaDataChanged ()));
		connect (Reply_.get (),
				SIGNAL (readyRead ()),
				this,
				SLOT (handleReadyRead ()));

		connect (Reply_.get (),
				Util::Overload<QNetworkReply::NetworkError> (&QNetworkReply::error),
				this,
				[this] (QNetworkReply::NetworkError err) { HandleError (MapError (err), Reply_->errorString ()); });
	}

	void Task::Stop ()
	{
		if (Reply_)
			Reply_->abort ();
	}

	void Task::ForbidNameChanges ()
	{
		CanChangeName_ = false;
	}

	QByteArray Task::Serialize () const
	{
		QByteArray result;
		{
			QDataStream out (&result, QIODevice::WriteOnly);
			out << 2
				<< URL_
				<< StartTime_
				<< Done_
				<< Total_
				<< Speed_
				<< CanChangeName_;
		}
		return result;
	}

	void Task::Deserialize (QByteArray& data)
	{
		QDataStream in (&data, QIODevice::ReadOnly);
		int version = 0;
		in >> version;
		if (version < 1 || version > 2)
			throw std::runtime_error ("Unknown version");

		in >> URL_
			>> StartTime_
			>> Done_
			>> Total_
			>> Speed_;

		if (version >= 2)
			in >> CanChangeName_;
	}

	double Task::GetSpeed () const
	{
		return Speed_;
	}

	qint64 Task::GetDone () const
	{
		return Done_;
	}

	qint64 Task::GetTotal () const
	{
		return Total_;
	}

	QString Task::GetState () const
	{
		if (!Reply_)
			return tr ("Stopped");
		else if (Done_ == Total_)
			return tr ("Finished");
		else
			return tr ("Running");
	}

	QString Task::GetURL () const
	{
		return Reply_ ? Reply_->url ().toString () : URL_.toString ();
	}

	int Task::GetTimeFromStart () const
	{
		return SpeedTimer_.elapsed ();
	}

	bool Task::IsRunning () const
	{
		return Reply_ && !URL_.isEmpty ();
	}

	QString Task::GetErrorString () const
	{
		return Reply_ ? Reply_->errorString () : tr ("Task isn't initialized properly");
	}

	QFuture<IDownload::Result> Task::GetFuture ()
	{
		return Promise_.future ();
	}

	void Task::Reset ()
	{
		RedirectHistory_.clear ();
		Done_ = -1;
		Total_ = 0;
		Speed_ = 0;
		FileSizeAtStart_ = -1;
		Reply_.reset ();
	}

	void Task::RestartTime ()
	{
		StartTime_ = QTime::currentTime ();
		SpeedTimer_.invalidate ();
		SpeedTimer_.start ();
	}

	void Task::RecalculateSpeed ()
	{
		Speed_ = static_cast<double> (Done_ * 1000) / static_cast<double> (SpeedTimer_.elapsed ());
	}

	void Task::HandleMetadataRedirection ()
	{
		const auto& newUrl = Reply_->rawHeader ("Location");
		if (!newUrl.size ())
			return;

		const auto code = Reply_->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ();
		if (code > 399 || code < 300)
		{
			qDebug () << Q_FUNC_INFO
					<< "there is a redirection URL, but the status code is not 3xx:"
					<< newUrl
					<< code
					<< Reply_->attribute (QNetworkRequest::HttpReasonPhraseAttribute);
			return;
		}

		if (!QUrl { newUrl }.isValid ())
		{
			qWarning () << Q_FUNC_INFO
				<< "invalid redirect URL"
				<< newUrl
				<< "for"
				<< Reply_->url ();
		}
		else if (RedirectHistory_.contains (newUrl))
		{
			qWarning () << Q_FUNC_INFO
				<< "redir loop detected"
				<< newUrl
				<< "for"
				<< Reply_->url ();
			HandleError (IDownload::Error::Type::ProtocolError, tr ("Redirections loop detected."));
		}
		else
		{
			RedirectHistory_ << newUrl;

			disconnect (Reply_.get (),
					0,
					this,
					0);

			QMetaObject::invokeMethod (this,
					"redirectedConstruction",
					Qt::QueuedConnection,
					Q_ARG (QByteArray, newUrl));
		}
	}

	namespace
	{
		QString GetFilenameAscii (const QString& contdis)
		{
			const QByteArray start { "filename=" };
			int startPos = contdis.indexOf (start) + start.size ();
			bool ignoreNextQuote = false;
			QString result;
			while (startPos < contdis.size ())
			{
				const auto cur = contdis.at (startPos++);
				if (cur == '\\')
				{
					ignoreNextQuote = true;
					continue;
				}

				result += cur;

				if (cur == '"' &&
						!ignoreNextQuote &&
						result.size () != 1)
					break;

				ignoreNextQuote = false;
			}
			return result;
		}

		QString GetFilenameUtf8 (const QString& contdis)
		{
			const QByteArray start = "filename*=UTF-8''";
			const auto markerPos = contdis.indexOf (start);
			if (markerPos == -1)
				return {};

			const auto startPos = markerPos + start.size ();
			auto endPos = contdis.indexOf (';', startPos);
			if (endPos == -1)
				endPos = contdis.size ();

			const auto& encoded = contdis.mid (startPos, endPos - startPos);
			const auto& utf8 = QByteArray::fromPercentEncoding (encoded.toLatin1 ());

			const auto& result = QString::fromUtf8 (utf8);
			return result;
		}

		QString GetFilename (const QString& contdis)
		{
			auto result = GetFilenameUtf8 (contdis);
			if (result.isEmpty ())
				result = GetFilenameAscii (contdis);

			if (result.startsWith ('"') && result.endsWith ('"'))
				result = result.mid (1, result.size () - 2);

			return result;
		}
	}

	void Task::HandleError (IDownload::Error::Type err, const QString& msg)
	{
		// TODO don't emit this when the file is already fully downloaded
		Util::ReportFutureResult (Promise_, IDownload::Result::Left ({ err, msg }));

		emit done (true);
	}

	void Task::HandleMetadataFilename ()
	{
		if (!CanChangeName_)
			return;

		const auto& contdis = Reply_->rawHeader ("Content-Disposition");
		qDebug () << Q_FUNC_INFO << contdis;
		if (!contdis.contains ("filename="))
			return;

		const auto& result = GetFilename (contdis);

		if (result.isEmpty ())
			return;

		auto path = To_->fileName ();
		auto oldPath = path;
		auto fname = QFileInfo (path).fileName ();
		path.replace (path.lastIndexOf (fname),
				fname.size (), result);

		if (path == oldPath)
		{
			qDebug () << Q_FUNC_INFO
					<< "new name equals to the old name, skipping renaming";
			return;
		}

		const auto openMode = To_->openMode ();
		To_->close ();

		if (!To_->rename (path))
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to rename to"
				<< path
				<< To_->errorString ();
		}
		if (!To_->open (openMode))
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to re-open the renamed file"
				<< path;
			To_->rename (oldPath);
			To_->open (openMode);
		}
	}

	void Task::handleDataTransferProgress (qint64 done, qint64 total)
	{
		Done_ = done;
		Total_ = total;

		RecalculateSpeed ();

		if (done == total)
			emit updateInterface ();
	}

	void Task::redirectedConstruction (const QByteArray& newUrl)
	{
		if (To_ && FileSizeAtStart_ >= 0)
		{
			To_->close ();
			To_->resize (FileSizeAtStart_);
			if (!To_->open (QIODevice::ReadWrite))
				qWarning () << Q_FUNC_INFO
						<< "failed to re-open the file after resizing"
						<< To_->fileName ()
						<< "to"
						<< FileSizeAtStart_
						<< ", error:"
						<< To_->errorString ();
		}

		Reply_.reset ();

		Referer_ = URL_;
		URL_ = QUrl::fromEncoded (newUrl);
		Start (To_);
	}

	void Task::handleMetaDataChanged ()
	{
		HandleMetadataRedirection ();
		HandleMetadataFilename ();
	}

	void Task::handleLocalTransfer ()
	{
		const auto& localFile = URL_.toLocalFile ();
		qDebug () << "LOCAL FILE" << localFile << To_->fileName ();

		auto reportError = [this] (const QString& msg)
		{
			QTimer::singleShot (0,
					this,
					[this, msg] { HandleError (IDownload::Error::Type::LocalError, msg); });
		};

		QFileInfo fi { localFile };
		if (!fi.isFile ())
		{
			qWarning () << Q_FUNC_INFO
					<< URL_
					<< "is not a file";
			reportError (tr ("Target path is not a file."));
			return;
		}

		const auto& destination = To_->fileName ();
		QFile file { localFile };
		To_->close ();
		if (!To_->remove () ||
				!file.copy (destination))
		{
			if (!To_->open (QIODevice::WriteOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open destfile"
						<< To_->fileName ()
						<< "for writing"
						<< To_->errorString ();
				reportError (tr ("Unable to open the destination file for writing."));
				return;
			}

			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open source file"
						<< file.fileName ()
						<< "for reading";
				reportError (tr ("Unable to open the source file for reading."));
				return;
			}

			const int chunkSize = 10 * 1024 * 1024;
			auto chunk = file.read (chunkSize);
			while (chunk.size ())
			{
				To_->write (chunk);
				chunk = file.read (chunkSize);
			}
		}

		QTimer::singleShot (0,
				this,
				SLOT (handleFinished ()));
	}

	bool Task::handleReadyRead ()
	{
		if (Reply_)
		{
			quint64 avail = Reply_->bytesAvailable ();
			quint64 res = To_->write (Reply_->readAll ());
			if (static_cast<quint64> (-1) == res ||
					res != avail)
			{
				qWarning () << Q_FUNC_INFO
						<< "Error writing to file:"
						<< To_->fileName ()
						<< To_->errorString ();

				const auto& errString = tr ("Error writing to file %1: %2")
						.arg (To_->fileName ())
						.arg (To_->errorString ());
				HandleError (IDownload::Error::Type::LocalError, errString);
			}
		}
		if (URL_.isEmpty () &&
				Core::Instance ().HasFinishedReply (Reply_.get ()))
		{
			handleFinished ();
			return true;
		}
		return false;
	}

	void Task::handleFinished ()
	{
		Util::ReportFutureResult (Promise_, IDownload::Result::Right ({}));

		emit done (false);
	}
}
}
