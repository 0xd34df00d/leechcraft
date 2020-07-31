/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPair>
#include "transcodingparams.h"

namespace LC
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

		void Enqueue (QStringList, const TranscodingParams&);
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
