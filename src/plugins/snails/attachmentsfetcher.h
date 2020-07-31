/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QFutureInterface>
#include <util/sll/eitherfwd.h>
#include "attdescr.h"
#include "accountthreadfwd.h"
#include "accountthreadworkerfwd.h"

class QTemporaryDir;

namespace LC
{
namespace Snails
{
	class Account;

	class AttachmentsFetcher
	{
		Account * const Acc_;
		const QStringList Folder_;
		const QByteArray MsgId_;

		QStringList AttQueue_;

		std::shared_ptr<QTemporaryDir> TempDir_;
		QStringList Paths_;
	public:
		struct TemporaryDirError {};

		struct FetchResult
		{
			std::shared_ptr<QTemporaryDir> TempDirGuard_;
			QStringList Paths_;
		};

		using Errors_t = AsInvokeError_t<AddErrors_t<FetchAttachmentResult_t::L_t, TemporaryDirError>>;
		using Result_t = Util::Either<Errors_t, FetchResult>;
	private:
		QFutureInterface<Result_t> Promise_;
	public:
		AttachmentsFetcher (Account*,
				const QStringList& folder, const QByteArray& msgId, const QStringList& attNames);

		QFuture<Result_t> GetFuture ();
	private:
		void RotateQueue ();
	};
}
}

