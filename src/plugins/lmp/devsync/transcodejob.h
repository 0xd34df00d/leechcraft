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
#include <QProcess>

namespace LeechCraft
{
namespace LMP
{
	struct TranscodingParams;

	class TranscodeJob : public QObject
	{
		Q_OBJECT

		QProcess *Process_;

		const QString OriginalPath_;
		QString TranscodedPath_;
		const QString TargetPattern_;
	public:
		TranscodeJob (const QString& path, const TranscodingParams& params, QObject* parent = 0);

		QString GetOrigPath () const;
		QString GetTranscodedPath () const;
		QString GetTargetPattern () const;
	private slots:
		void handleFinished (int, QProcess::ExitStatus);
		void handleReadyRead ();
	signals:
		void done (TranscodeJob*, bool);
	};
}
}
