/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "attachmentsfetcher.h"
#include <QTemporaryDir>
#include <util/threads/futures.h>
#include <util/sll/visitor.h>
#include "account.h"

namespace LC
{
namespace Snails
{
	AttachmentsFetcher::AttachmentsFetcher (Account *acc,
			const QStringList& folder, const QByteArray& msgId, const QStringList& attNames)
	: Acc_ { acc }
	, Folder_ { folder }
	, MsgId_ { msgId }
	, AttQueue_ { attNames }
	{
		Promise_.reportStarted ();

		RotateQueue ();
	}

	QFuture<AttachmentsFetcher::Result_t> AttachmentsFetcher::GetFuture ()
	{
		return Promise_.future ();
	}

	void AttachmentsFetcher::RotateQueue ()
	{
		if (AttQueue_.isEmpty ())
		{
			Util::ReportFutureResult (Promise_, Result_t::Right ({ TempDir_, Paths_ }));
			return;
		}

		if (!TempDir_)
		{
			TempDir_ = std::make_shared<QTemporaryDir> ();
			if (!TempDir_->isValid ())
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to create temporary directory";
				Util::ReportFutureResult (Promise_, Result_t::Left (TemporaryDirError {}));
				return;
			}
		}

		const auto& attName = AttQueue_.takeFirst ();
		const auto& filePath = QDir { TempDir_->path () }.filePath (attName);
		Util::Sequence (Acc_, Acc_->FetchAttachment (Folder_, MsgId_, attName, filePath)) >>
				Util::Visitor
				{
					[=] (Util::Void)
					{
						Paths_ << filePath;
						RotateQueue ();
					},
					[=] (auto e) { Util::ReportFutureResult (Promise_, Result_t::LeftLift (std::move (e))); }
				};
}
}
}
