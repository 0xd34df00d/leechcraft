/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QTime>
#include <QSet>

namespace LeechCraft
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
			const QString SourceFile_;
			const QString TargetFile_;
			const int Index_;
			const QTime From_;
			const QTime To_;

			const QString Artist_;
			const QString Album_;
			const QString Title_;
			const int Date_;
			const QString Genre_;
		};
		QList<SplitQueueItem> SplitQueue_;

		QSet<QString> EmittedErrors_;
	public:
		CueSplitter (const QString& cue, const QString& dir, QObject* = 0);
	private slots:
		void split ();
		void scheduleNext ();
		void handleProcessFinished (int);
		void handleProcessError ();
	signals:
		void error (const QString&);
		void finished ();
	};
}
}
}
