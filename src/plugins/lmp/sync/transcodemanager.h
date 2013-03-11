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
#include <QPair>
#include "transcodingparams.h"

namespace LeechCraft
{
namespace LMP
{
	class TranscodeJob;

	class TranscodeManager : public QObject
	{
		Q_OBJECT

		QList<QPair<QString, TranscodingParams>> Queue_;

		QList<TranscodeJob*> RunningJobs_;
	public:
		TranscodeManager (QObject* = 0);

		void Enqueue (const QStringList&, const TranscodingParams&);
	private:
		void EnqueueJob (const QPair<QString, TranscodingParams>&);
	private slots:
		void handleDone (TranscodeJob*, bool);
	signals:
		void fileStartedTranscoding (const QString& origPath);
		void fileReady (const QString& origPath,
				const QString& transcodedPath, const QString& pattern);
		void fileFailed (const QString&);
	};
}
}
