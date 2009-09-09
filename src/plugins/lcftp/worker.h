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

#ifndef PLUGINS_LCFTP_WORKER_H
#define PLUGINS_LCFTP_WORKER_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QFile>
#include <QBuffer>
#include <QUrl>
#include <QDateTime>
#include <curl/curl.h>
#include "structures.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct Wrapper;
			typedef boost::shared_ptr<Wrapper> Wrapper_ptr;

			typedef boost::shared_ptr<CURL> CURL_ptr;

			class Worker : public QObject
			{
				Q_OBJECT

				friend struct Wrapper;
				int ID_;
				CURL_ptr Handle_;
				Wrapper_ptr W_;
				boost::shared_ptr<QFile> File_;
				boost::shared_ptr<QBuffer> ListBuffer_;
				bool IsWorking_;
				bool Paused_;
				quint64 DLNow_,
					DLTotal_,
					ULNow_,
					ULTotal_,
					InitialSize_,
					DownLimit_,
					UpLimit_;
				QDateTime StartDT_;
				TaskData Task_;
			public:
				struct TaskState
				{
					int WorkID_;
					bool IsWorking_;
					QUrl URL_;
					QPair<quint64, quint64> DL_;
					QPair<quint64, quint64> UL_;
					quint64 DLSpeed_;
					quint64 ULSpeed_;
					TaskData::Direction Direction_;
					bool Paused_;
				};

				Worker (int, QObject* = 0);
				virtual ~Worker ();

				bool IsWorking () const;
				void SetID (int);
				TaskState GetState () const;
				TaskData GetTask () const;
				QUrl GetURL () const;
				CURL_ptr GetHandle () const;
				/** Prepares to perform the given task, starts it and
				 * returns immediately.
				 * 
				 * @param[in] task The task description.
				 */
				CURL_ptr Start (const TaskData& task);
				/** This function is used by the Core to notify this
				 * worker that it has finished and now should do some
				 * post-processing, if any.
				 *
				 * @param[in] result The result of the operation.
				 */
				void NotifyFinished (CURLcode result);
				void Abort ();
				void Pause ();
				void Resume ();
				qint64 GetDownLimit () const;
				qint64 GetUpLimit () const;
				void SetDownLimit (qint64);
				void SetUpLimit (qint64);
				QString GetLog () const;
			private:
				void HandleTask (const TaskData&, CURL_ptr);
				void ParseBuffer (const TaskData&);
				void Reset ();
				size_t WriteData (void*, size_t, size_t);
				size_t ReadData (char*, size_t, size_t);
				size_t ListDir (void*, size_t, size_t);
				int Progress (double, double, double, double);
				void UpdateHandleSettings (CURL_ptr);
			signals:
				void error (const QString&, const TaskData&);
				void finished (const TaskData&);
				void fetchedEntry (const FetchedEntry&);
			};

			typedef boost::shared_ptr<Worker> Worker_ptr;
		};
	};
};

#endif

