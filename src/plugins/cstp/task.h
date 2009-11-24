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

#ifndef PLUGINS_CSTP_TASK_H
#define PLUGINS_CSTP_TASK_H
#include <list>
#include <boost/intrusive_ptr.hpp>
#include <memory>
#include <QObject>
#include <QUrl>
#include <QTime>
#include <QNetworkReply>
#include <QStringList>
#include "morphfile.h"

class QAuthenticator;
class QNetworkProxy;
class QIODevice;
class QFile;
class QTimer;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class Hook;

			class Task : public QObject
			{
				Q_OBJECT
				std::auto_ptr<QNetworkReply> Reply_;
				QUrl URL_;
				QTime StartTime_;
				qint64 Done_, Total_, FileSizeAtStart_;
				double Speed_;
				QList<QByteArray> RedirectHistory_;
				boost::intrusive_ptr<MorphFile> To_;
				int Counter_;
				int UpdateCounter_;
				QTimer *Timer_;
			public:
				explicit Task (const QUrl& = QUrl ());
				explicit Task (QNetworkReply*);
				void Start (const boost::intrusive_ptr<MorphFile>&);
				void Stop ();

				QByteArray Serialize () const;
				void Deserialize (QByteArray&);

				double GetSpeed () const;
				qint64 GetDone () const;
				qint64 GetTotal () const;
				QString GetState () const;
				QString GetURL () const;
				int GetTimeFromStart () const;
				bool IsRunning () const;
				QString GetErrorString () const;

				void AddRef ();
				void Release ();
			private:
				void Reset ();
				void RecalculateSpeed ();
			private slots:
				void handleDataTransferProgress (qint64, qint64);
				void redirectedConstruction (const QByteArray&);
				void handleMetaDataChanged ();
				/** Returns true if the reply is at end after this read.
				 */
				bool handleReadyRead ();
				void handleFinished ();
				void handleError ();
			signals:
				void updateInterface ();
				void done (bool);
			};

			void intrusive_ptr_add_ref (Task*);
			void intrusive_ptr_release (Task*);
		};
	};
};

#endif

