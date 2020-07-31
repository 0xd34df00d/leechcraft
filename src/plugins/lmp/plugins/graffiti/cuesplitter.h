/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QTime>
#include <QSet>

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	class CueSplitter : public QObject
	{
		Q_OBJECT

		const QString CueFile_;
		const QString Dir_;

		struct SplitQueueItem
		{
			QString SourceFile_;
			QString TargetFile_;
			int Index_;
			QTime From_;
			QTime To_;

			QString Artist_;
			QString Album_;
			QString Title_;
			int Date_;
			QString Genre_;
			QString DiscId_;
		};
		QList<SplitQueueItem> SplitQueue_;

		int TotalItems_ = 0;
		int CurrentlyProcessing_ = 0;

		QSet<QString> EmittedErrors_;
	public:
		CueSplitter (const QString& cue, const QString& dir, QObject* = 0);

		QString GetCueFile () const;
	private slots:
		void split ();
		void scheduleNext ();
		void handleProcessFinished (int);
		void handleProcessError ();
	signals:
		void error (const QString&);
		void finished (CueSplitter*);

		void splitProgress (int, int, CueSplitter*);
	};
}
}
}
