/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>

namespace LC
{
namespace LMP
{
	class ISyncPlugin;
	class TranscodeManager;
	struct TranscodingParams;

	class SyncManagerBase : public QObject
	{
		Q_OBJECT

	protected:
		TranscodeManager *Transcoder_;

		int TranscodedCount_;
		int TotalTCCount_;
		bool WereTCErrors_;

		int CopiedCount_;
		int TotalCopyCount_;
	public:
		SyncManagerBase (QObject* = 0);
	protected:
		void AddFiles (const QStringList&, const TranscodingParams&);
		void HandleFileTranscoded (const QString&, const QString&);
	private:
		void CheckTCFinished ();
		void CheckUploadFinished ();
	protected slots:
		void handleStartedTranscoding (const QString&);
		virtual void handleFileTranscoded (const QString&, const QString&, QString) = 0;
		void handleFileTCFailed (const QString&);
		void handleStartedCopying (const QString&);
		void handleFinishedCopying ();
		void handleCopyProgress (qint64, qint64);
		void handleErrorCopying (const QString&, const QString&);
	signals:
		void uploadLog (const QString&);

		void transcodingProgress (int, int, SyncManagerBase*);
		void uploadProgress (int, int, SyncManagerBase*);
		void singleUploadProgress (int, int, SyncManagerBase*);
	};
}
}
