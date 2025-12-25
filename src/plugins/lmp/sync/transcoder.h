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
#include <util/sll/either.h>
#include <util/threads/coro/channel.h>
#include <util/threads/coro/taskfwd.h>
#include "transcodingparams.h"

namespace LC::LMP
{
	class Transcoder : public QObject
	{
		const TranscodingParams Params_;

		Util::Channel<QString> ToTranscode_;
	public:
		struct Result
		{
			QString OrigPath_;

			struct Failure
			{
				QProcess::ExitStatus ExitStatus_;
				QString Reason_;
			};

			struct Success
			{
				QString TargetPath_;
			};
			Util::Either<Failure, Success> Transcoded_;
		};
	private:
		Util::Channel<Result> Results_;
	public:
		explicit Transcoder (const QStringList& files, const TranscodingParams& params);

		Util::Channel<Result>& GetResults ();
	private:
		Util::ContextTask<void> TranscodeFile (const QString& origPath);
	};
}
