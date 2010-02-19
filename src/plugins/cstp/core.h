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

#ifndef PLUGINS_CSTP_CORE_H
#define PLUGINS_CSTP_CORE_H
#include <vector>
#include <boost/intrusive_ptr.hpp>
#include <QAbstractItemModel>
#include <QStringList>
#include <QNetworkProxy>
#include <QNetworkAccessManager>
#include <QSet>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

class QFile;

namespace boost
{
	namespace logic
	{
		class tribool;
	};
};

namespace LeechCraft
{
	namespace Util
	{
		class HistoryModel;
	};

	namespace Plugins
	{
		namespace CSTP
		{
			class Task;
			class HistoryModel;
			class MorphFile;

			namespace _Local
			{
				struct ObjectFinder;
				struct SpeedAccumulator;
			};

			class Core : public QAbstractItemModel
			{
				Q_OBJECT
				QStringList Headers_;

				friend struct _Local::ObjectFinder;
				friend struct _Local::SpeedAccumulator;

				struct TaskDescr
				{
					boost::intrusive_ptr<Task> Task_;
					boost::intrusive_ptr<MorphFile> File_;
					QString Comment_;
					bool ErrorFlag_;
					LeechCraft::TaskParameters Parameters_;
					quint32 ID_;
					QStringList Tags_;
				};
				typedef std::vector<TaskDescr> tasks_t;
				tasks_t ActiveTasks_;
				bool SaveScheduled_;
				QNetworkAccessManager *NetworkAccessManager_;
				QToolBar *Toolbar_;
				QSet<QNetworkReply*> FinishedReplies_;
				QModelIndex Selected_;
				ICoreProxy_ptr CoreProxy_;

				explicit Core ();
			public:
				enum
				{
					HURL
					, HState
					, HProgress
					, HSpeed
					, HRemaining
					, HDownloading
				};

				virtual ~Core ();
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetCoreProxy () const;
				void SetToolbar (QToolBar*);
				void ItemSelected (const QModelIndex&);

				int AddTask (LeechCraft::DownloadEntity&);
				void KillTask (int);
				qint64 GetDone (int) const;
				qint64 GetTotal (int) const;
				bool IsRunning (int) const;
				qint64 GetTotalDownloadSpeed () const;
				bool CouldDownload (const LeechCraft::DownloadEntity&);
				QAbstractItemModel* GetRepresentationModel ();
				void SetNetworkAccessManager (QNetworkAccessManager*);
				QNetworkAccessManager* GetNetworkAccessManager () const;
				bool HasFinishedReply (QNetworkReply*) const;
				void RemoveFinishedReply (QNetworkReply*);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			public slots:
				void removeTriggered (int = -1);
				void removeAllTriggered ();
				void startTriggered (int = -1);
				void stopTriggered (int = -1);
				void startAllTriggered ();
				void stopAllTriggered ();
			private slots:
				void done (bool); 
				void updateInterface ();
				void writeSettings ();
				void finishedReply (QNetworkReply*);
			private:
				int AddTask (const QUrl&,
						const QString&,
						const QString&,
						const QString&,
						const QStringList&,
						LeechCraft::TaskParameters = LeechCraft::NoParameters);
				int AddTask (QNetworkReply*,
						const QString&,
						const QString&,
						const QString&,
						const QStringList&,
						LeechCraft::TaskParameters = LeechCraft::NoParameters);
				int AddTask (TaskDescr&,
						const QString&,
						const QString&,
						const QString&,
						const QStringList&,
						LeechCraft::TaskParameters = LeechCraft::NoParameters);
				void ReadSettings ();
				void ScheduleSave ();
				tasks_t::const_iterator FindTask (QObject*) const;
				tasks_t::iterator FindTask (QObject*);
				void Remove (tasks_t::iterator);
				void AddToHistory (tasks_t::const_iterator);
				tasks_t::const_reference TaskAt (int) const;
				tasks_t::reference TaskAt (int);
			signals:
				void taskFinished (int);
				void taskRemoved (int);
				void taskError (int, IDownload::Error);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void notify (const LeechCraft::Notification&);
				void error (const QString&);
				void fileExists (boost::logic::tribool*);
			};
		};
	};
};

#endif

