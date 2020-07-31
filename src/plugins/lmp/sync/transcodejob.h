/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QProcess>

namespace LC
{
namespace LMP
{
	struct TranscodingParams;

	class TranscodeJob : public QObject
	{
		Q_OBJECT

		QProcess *Process_;

		const QString OriginalPath_;
		const QString TranscodedPath_;
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
