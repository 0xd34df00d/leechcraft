#include "core.h"
#include <QUrl>
#include <QMutexLocker>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <curl/curl.h>
#include <plugininterface/proxy.h>
#include "inactiveworkersfilter.h"

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
				WorkersFilter_.reset (new InactiveWorkersFilter (this));
				curl_global_init (CURL_GLOBAL_ALL);
				for (int i = 0; i < 8; ++i)
				{
					Worker_ptr w (new Worker (i));
					w->start ();
					connect (w.get (),
							SIGNAL (error (const QString&)),
							this,
							SLOT (handleError (const QString&)),
							Qt::QueuedConnection);
					Workers_ << w;
					States_ << Workers_.at (i)->GetState ();
				}

				handleUpdateInterface ();

				QTimer *timer = new QTimer (this);
				timer->setInterval (500);
				connect (timer,
						SIGNAL (timeout ()),
						this,
						SLOT (handleUpdateInterface ()));
				timer->start ();
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

			QAbstractItemModel* Core::GetModel () const
			{
				return WorkersFilter_.get ();
			}

			int Core::columnCount (const QModelIndex&) const
			{
				return 3;
			}

			QVariant Core::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();

				int working = Workers_.size ();
				int r = index.row ();
				int c = index.column ();

				if (role == Qt::DisplayRole)
				{
					switch (c)
					{
						case 0:
							if (r < working)
								return States_.at (r).URL_.toString ();
							else
								return Tasks_ [r - working].URL_.toString ();
						case 1:
							return r < working ? tr ("Downloading") : tr ("Waiting");
						case 2:
							if (r >= working)
								return QVariant ();
							else
							{
								QPair<quint64, quint64> s = States_.at (r).DL_;
								if (s.second)
									return tr ("%1 of %2 (%3%)")
										.arg (Util::Proxy::Instance ()->MakePrettySize (s.first))
										.arg (Util::Proxy::Instance ()->MakePrettySize (s.second))
										.arg (s.first * 100 / s.second);
								else
									return tr ("%1 of %2")
										.arg (Util::Proxy::Instance ()->MakePrettySize (s.first))
										.arg (Util::Proxy::Instance ()->MakePrettySize (s.second));
							}
						default:
							return QVariant ();
					}
				}
				else
					return QVariant ();
			}

			QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}

			QModelIndex Core::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}

			int Core::rowCount (const QModelIndex& parent) const
			{
				if (parent.isValid ())
					return 0;

				int result = 0;
				result += Workers_.size ();
				result += Tasks_.size ();
				return result;
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
				QueueTask (td);
				WorkerWait_.wakeOne ();
				return td.ID_;
			}

			bool Core::IsAcceptable (int index) const
			{
				return index >= States_.size () ||
					States_.at (index).IsWorking_;
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
					beginRemoveRows (QModelIndex (), 0, 0);
					TaskData td = Tasks_.takeFirst ();
					TasksLock_.unlock ();
					endRemoveRows ();
					QTimer::singleShot (0,
							this,
							SLOT (handleUpdateInterface ()));
					return td;
				}
				else
				{
					TaskData td;
					return td;
				}
			}

			void Core::FinishedTask (int id)
			{
				WorkerWaitMutex_.unlock ();
				if (id >= 0)
					Proxy_->FreeID (id);
				QTimer::singleShot (0,
						this,
						SLOT (handleUpdateInterface ()));
			}

			void Core::QueueTask (const TaskData& td)
			{
				beginInsertRows (QModelIndex (),
						Tasks_.size (), Tasks_.size ());
				TasksLock_.lockForWrite ();
				Tasks_ << td;
				TasksLock_.unlock ();
				endInsertRows ();
			}
			
			void Core::handleError (const QString& msg)
			{
				QMessageBox::critical (0,
						tr ("LeechCraft"),
						msg);
			}

			void Core::handleUpdateInterface ()
			{
				States_.clear ();
				for (int i = 0; i < Workers_.size (); ++i)
					States_ << Workers_.at (i)->GetState ();

				emit dataChanged (index (0, 0),
						index (States_.size () - 1, 2));
			}
		};
	};
};

