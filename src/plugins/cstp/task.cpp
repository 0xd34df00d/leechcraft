#include "task.h"
#include <algorithm>
#include <typeinfo>
#include <stdexcept>
#include <QUrl>
#include <QHttp>
#include <QFtp>
#include <QFileInfo>
#include <QDataStream>
#include <QDir>
#include <QtDebug>
#include "hook.h"
#include "core.h"
#include "xmlsettingsmanager.h"

Task::Task (const QString& str)
: URL_ (str)
, Done_ (-1)
, Total_ (0)
, FileSizeAtStart_ (-1)
, Speed_ (0)
{
	StartTime_.start ();
}

Task::~Task ()
{
}

void Task::Start (const boost::shared_ptr<QFile>& tof)
{
	FileSizeAtStart_ = tof->size ();
	Start (tof.get ());
}

void Task::Stop ()
{
	Reply_->abort ();
}

QByteArray Task::Serialize () const
{
	QByteArray result;
	{
		QDataStream out (&result, QIODevice::WriteOnly);
		out << 1
			<< URL_
			<< StartTime_
			<< Done_
			<< Total_
			<< Speed_;
	}
	return result;
}

void Task::Deserialize (QByteArray& data)
{
	QDataStream in (&data, QIODevice::ReadOnly);
	int version = 0;
	in >> version;
	if (version == 1)
	{
		in >> URL_
			>> StartTime_
			>> Done_
			>> Total_
			>> Speed_;
	}
	else
		throw std::runtime_error ("Unknown version");
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
	if (!Reply_.get ())
		return tr ("Stopped");
	else if (Done_ == Total_)
		return tr ("Finished");
	else
		return tr ("Running");
}

QString Task::GetURL () const
{
	return URL_.toString ();
}

int Task::GetTimeFromStart () const
{
	return StartTime_.elapsed ();
}

bool Task::IsRunning () const
{
	return Reply_.get ();
}

QString Task::GetErrorString () const
{
	// TODO implement own translations for errors.
	return Reply_.get () ? Reply_->errorString () : tr ("Task isn't initialized properly");
}

void Task::Start (QIODevice *to)
{
	To_ = to;

	QString ua = XmlSettingsManager::Instance ()
		.property ("UserUserAgent").toString ();
	if (ua.isEmpty ())
		ua = XmlSettingsManager::Instance ()
			.property ("PredefinedUserAgent").toString ();

	QNetworkRequest req (URL_);
	req.setRawHeader ("Range", QString ("bytes=%1-").arg (to->size ()).toLatin1 ());
	req.setRawHeader ("User-Agent", ua.toLatin1 ());
	req.setRawHeader ("Referer", QString (QString ("http://") + URL_.host ()).toLatin1 ());

	StartTime_.restart ();
	QNetworkAccessManager *nam = Core::Instance ().GetNetworkAccessManager ();
	Reply_.reset (nam->get (req));
	connect (Reply_.get (),
			SIGNAL (downloadProgress (qint64, qint64)),
			this,
			SLOT (handleDataTransferProgress (qint64, qint64)));
	connect (Reply_.get (),
			SIGNAL (finished ()),
			this,
			SLOT (handleFinished ()));
	connect (Reply_.get (),
			SIGNAL (error (QNetworkReply::NetworkError)),
			this,
			SLOT (handleError ()));
	connect (Reply_.get (),
			SIGNAL (metaDataChanged ()),
			this,
			SLOT (handleMetaDataChanged ()));
	connect (Reply_.get (),
			SIGNAL (readyRead ()),
			this,
			SLOT (handleReadyRead ()));
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

void Task::RecalculateSpeed ()
{
	Speed_ = static_cast<double> (Done_ * 1000) / static_cast<double> (StartTime_.elapsed ());
}

void Task::handleDataTransferProgress (qint64 done, qint64 total)
{
	Done_ = done;
	Total_ = total;

	RecalculateSpeed ();

	emit updateInterface ();
}

void Task::redirectedConstruction (QIODevice *to, const QString& newUrl)
{
	QFile *file = dynamic_cast<QFile*> (to);
	if (file && FileSizeAtStart_ >= 0)
	{
		file->close ();
		file->size ();
		file->resize (FileSizeAtStart_);
		file->open (QIODevice::ReadWrite);
	}

	URL_ = newUrl;
	Start (file);
}

void Task::handleMetaDataChanged ()
{
	QVariant locHeader = Reply_->header (QNetworkRequest::LocationHeader);
	if (locHeader.isValid ())
	{
		QString newUrl = locHeader.toString ();
		if (!QUrl (newUrl).isValid () ||
				RedirectHistory_.contains (newUrl, Qt::CaseInsensitive))
		{
			qDebug () << Q_FUNC_INFO << "redirection failed, possibly a loop detected" << newUrl;
			emit done (true);
		}
		else
		{
			RedirectHistory_ << newUrl;
		
			QMetaObject::invokeMethod (this,
					"redirectedConstruction",
					Qt::QueuedConnection,
					Q_ARG (QIODevice*, To_),
					Q_ARG (QString, newUrl));
		}

		Reply_->blockSignals (true);
		Reply_->abort ();
		Reply_.release ()->deleteLater ();
	}
}

void Task::handleReadyRead ()
{
	To_->write (Reply_->readAll ());
}

void Task::handleFinished ()
{
	Reply_.release ()->deleteLater ();
	emit done (false);
}

void Task::handleError ()
{
	emit done (true);
}

