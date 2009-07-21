#include "core.h"
#include <QUrl>
#include <QMutexLocker>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <curl/curl.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Core* Core::Instance_ = 0;
			QMutex Core::InstanceMutex_;

			Core::Core ()
			: Quitting_ (false)
			{
				curl_global_init (CURL_GLOBAL_ALL);
				for (int i = 0; i < 8; ++i)
				{
					Worker_ptr w (new Worker);
					w->start ();
					connect (w.get (),
							SIGNAL (error (const QString&)),
							this,
							SLOT (handleError (const QString&)),
							Qt::QueuedConnection);
					Workers_ << w;
				}
			}

			Core& Core::Instance ()
			{
				if (!Instance_)
				{
					QMutexLocker locker (&InstanceMutex_);
					if (!Instance_)
						Instance_ = new Core ();
				}
				return *Instance_;
			}

			void Core::Release ()
			{
				Quitting_ = true;
				Q_FOREACH (Worker_ptr w, Workers_)
					w->SetExit ();
				WorkerWait_.wakeAll ();

				curl_global_cleanup ();
				deleteLater ();
				Q_FOREACH (Worker_ptr w, Workers_)
				{
					w->wait (10);
					w->terminate ();
				}
			}

			void Core::SetCoreProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			QStringList Core::Provides () const
			{
				curl_version_info_data *data = curl_version_info (CURLVERSION_NOW);
				QStringList result ("ftp");
				if (data->age > 0)
				{
					if (data->features & CURL_VERSION_SSL)
						result << "ftps";
				}
				return result;
			}

			bool Core::IsOK (const DownloadEntity& e) const
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return false;

				QUrl url = e.Entity_.toUrl ();
				if (Provides ().contains (url.scheme ()))
					return true;
				else
					return false;
			}

			int Core::Add (DownloadEntity e)
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return -1;

				QUrl url = e.Entity_.toUrl ();
				QString path = e.Location_;

				QFileInfo fi (path);
				QString dir = fi.dir ().path (),
						file = fi.fileName ();
	
				if (!(e.Parameters_ & LeechCraft::Internal))
				{
					if (fi.isDir ())
					{
						dir = e.Location_;
						file = QFileInfo (url.toString (QUrl::RemoveFragment)).fileName ();
						if (file.isEmpty ())
							file = "index";
					}
					else if (fi.isFile ());
					else
						return -1;
				}

				TaskData td =
				{
					Proxy_->GetID (),
					url,
					dir + "/" + file
				};
				TasksLock_.lockForWrite ();
				Tasks_ << td;
				TasksLock_.unlock ();
				WorkerWait_.wakeOne ();
				return td.ID_;
			}

			TaskData Core::GetNextTask ()
			{
				TasksLock_.lockForRead ();
				while (!Tasks_.size ())
				{
					TasksLock_.unlock ();
					WorkerWaitMutex_.lock ();
					WorkerWait_.wait (&WorkerWaitMutex_);
					if (Quitting_)
						break;
					TasksLock_.lockForWrite ();
				}

				if (!Quitting_)
				{
					TaskData td = Tasks_.takeFirst ();
					TasksLock_.unlock ();
					return td;
				}
				else
				{
					TaskData td;
					return td;
				}
			}

			void Core::FinishedTask ()
			{
				WorkerWaitMutex_.unlock ();
			}
			
			void Core::handleError (const QString& msg)
			{
				QMessageBox::critical (0,
						tr ("LeechCraft"),
						msg);
			}
		};
	};
};

