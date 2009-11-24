/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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
#include <QTimer>
#include <QtDebug>
#include "hook.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			Task::Task (const QUrl& url)
			: URL_ (url)
			, Done_ (-1)
			, Total_ (0)
			, FileSizeAtStart_ (-1)
			, Speed_ (0)
			, Counter_ (0)
			, Timer_ (0)
			{
				StartTime_.start ();
			}
			
			Task::Task (QNetworkReply *reply)
			: Reply_ (reply)
			, Done_ (-1)
			, Total_ (0)
			, FileSizeAtStart_ (-1)
			, Speed_ (0)
			, Timer_ (0)
			{
				StartTime_.start ();
			}

			void Task::Start (const boost::intrusive_ptr<MorphFile>& tof)
			{
				FileSizeAtStart_ = tof->size ();
				To_ = tof;
			
				if (!Reply_.get ())
				{
					QString ua = XmlSettingsManager::Instance ()
						.property ("UserUserAgent").toString ();
					if (ua.isEmpty ())
						ua = XmlSettingsManager::Instance ()
							.property ("PredefinedUserAgent").toString ();
			
					QNetworkRequest req (URL_);
					req.setRawHeader ("Range", QString ("bytes=%1-").arg (tof->size ()).toLatin1 ());
					req.setRawHeader ("User-Agent", ua.toLatin1 ());
					req.setRawHeader ("Referer", QString (QString ("http://") + URL_.host ()).toLatin1 ());
			
					StartTime_.restart ();
					QNetworkAccessManager *nam = Core::Instance ().GetNetworkAccessManager ();
					Reply_.reset (nam->get (req));
				}
				else
				{
					qint64 contentLength = Reply_->
						header (QNetworkRequest::ContentLengthHeader).toInt ();
					if (contentLength &&
							Reply_->bytesAvailable () == contentLength)
					{
						handleReadyRead ();
						handleFinished ();
						return;
					}
					else if (!Reply_->isOpen ())
					{
						handleError ();
						return;
					}
					else
					{
						if (handleReadyRead ())
							return;
					}
				}
			
				Reply_->setParent (0);
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
			
			void Task::Stop ()
			{
				if (Reply_.get ())
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
				return Reply_.get () ? Reply_->url ().toString () : URL_.toString ();
			}
			
			int Task::GetTimeFromStart () const
			{
				return StartTime_.elapsed ();
			}
			
			bool Task::IsRunning () const
			{
				return Reply_.get () && !URL_.isEmpty ();
			}
			
			QString Task::GetErrorString () const
			{
				// TODO implement own translations for errors.
				return Reply_.get () ? Reply_->errorString () : tr ("Task isn't initialized properly");
			}
			
			void Task::AddRef ()
			{
				++Counter_;
			}
			
			void Task::Release ()
			{
				--Counter_;
				if (!Counter_)
					deleteLater ();
			}
			
			void Task::Reset ()
			{
				RedirectHistory_.clear ();
				Done_ = -1;
				Total_ = 0;
				Speed_ = 0;
				FileSizeAtStart_ = -1;
				Reply_.reset ();

				delete Timer_;
				Timer_ = new QTimer (this);
				connect (Timer_,
						SIGNAL (timeout ()),
						this,
						SIGNAL (updateInterface ()));
				Timer_->start (3000);
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
			
				if (done == total)
					emit updateInterface ();
			}
			
			void Task::redirectedConstruction (const QByteArray& newUrl)
			{
				if (To_ && FileSizeAtStart_ >= 0)
				{
					To_->close ();
					To_->size ();
					To_->resize (FileSizeAtStart_);
					To_->open (QIODevice::ReadWrite);
				}
			
				Reply_.reset ();
			
				URL_ = QUrl::fromEncoded (newUrl);
				Start (To_);
			}
			
			void Task::handleMetaDataChanged ()
			{
				QByteArray newUrl = Reply_->rawHeader ("Location");
				if (newUrl.size ())
				{
					if (!QUrl (newUrl).isValid ())
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
						emit done (true);
					}
					else
					{
						RedirectHistory_ << newUrl;
					
						QMetaObject::invokeMethod (this,
								"redirectedConstruction",
								Qt::QueuedConnection,
								Q_ARG (QByteArray, newUrl));
					}
				}
			}
			
			bool Task::handleReadyRead ()
			{
				if (Reply_.get ())
					To_->write (Reply_->readAll ());
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
				Core::Instance ().RemoveFinishedReply (Reply_.get ());
				disconnect (Reply_.get (),
						0,
						this,
						0);
				if (Reply_.get ())
					Reply_.release ()->deleteLater ();
				emit done (false);
			}
			
			void Task::handleError ()
			{
				emit done (true);
			}
			
			void intrusive_ptr_add_ref (Task *task)
			{
				task->AddRef ();
			}
			
			void intrusive_ptr_release (Task *task)
			{
				task->Release ();
			}
		};
	};
};

