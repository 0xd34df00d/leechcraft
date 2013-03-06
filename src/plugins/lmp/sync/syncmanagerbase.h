/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QObject>
#include <QMap>

namespace LeechCraft
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
	signals:
		void uploadLog (const QString&);

		void transcodingProgress (int, int);
		void uploadProgress (int, int);
	};
}
}
