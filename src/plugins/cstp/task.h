/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <QObject>
#include <QUrl>
#include <QElapsedTimer>
#include <QNetworkReply>
#include <QStringList>
#include <QFutureInterface>
#include <util/sll/either.h>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>

class QAuthenticator;
class QNetworkProxy;
class QIODevice;
class QFile;
class QTimer;

namespace LC
{
namespace CSTP
{
	class Task : public QObject
	{
		Q_OBJECT

		std::unique_ptr<QNetworkReply, std::function<void (QNetworkReply*)>> Reply_;
		QUrl URL_;
		QTime StartTime_;
		QElapsedTimer SpeedTimer_;
		qint64 Done_ = -1, Total_ = 0, FileSizeAtStart_ = -1;
		double Speed_ = 0;
		QList<QUrl> RedirectHistory_;
		std::shared_ptr<QFile> To_;
		QTimer *Timer_;
		bool CanChangeName_ = true;

		QUrl Referer_;

		const QNetworkAccessManager::Operation Operation_;

		const QVariantMap Headers_;

		const QByteArray UploadData_ = {};

		QFutureInterface<IDownload::Result> Promise_;
	public:
		explicit Task (const QUrl& url = QUrl (), const QVariantMap& params = QVariantMap ());
		explicit Task (QNetworkReply*);

		void Start (const std::shared_ptr<QFile>&);
		void Stop ();
		void ForbidNameChanges ();

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

		QFuture<IDownload::Result> GetFuture ();
	private:
		void Reset ();
		void RestartTime ();
		void RecalculateSpeed ();
		void HandleMetadataRedirection ();
		void HandleMetadataFilename ();

		void HandleError (IDownload::Error::Type, const QString&);
	private slots:
		void handleDataTransferProgress (qint64, qint64);
		void redirectedConstruction (const QUrl&);
		void handleMetaDataChanged ();
		void handleLocalTransfer ();
		/** Returns true if the reply is at end after this read.
			*/
		bool handleReadyRead ();
		void handleFinished ();
	signals:
		void updateInterface ();
		void done (bool);
	};
}
}
