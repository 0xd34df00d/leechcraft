/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "threadpool.h"
#include <QTimer>
#include <util/sll/visitor.h>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include <util/threads/monadicfuture.h>
#include "accountthread.h"
#include "accountthreadworker.h"

namespace LC
{
namespace Snails
{
	ThreadPool::ThreadPool (Account *acc, Storage *st, QObject *parent)
	: QObject { parent }
	, Acc_ { acc }
	, Storage_ { st }
	{
	}

	QFuture<EitherInvokeError_t<Util::Void>> ThreadPool::TestConnectivity ()
	{
		auto thread = CreateThread ();
		auto future = thread->Schedule (TaskPriority::High, &AccountThreadWorker::TestConnectivity);
		Util::Sequence (nullptr, future) >>
				[thread] (const auto&) {};
		return future;
	}

	void ThreadPool::RunThreads ()
	{
		if (CheckingNext_)
			return;

		if (HitLimit_)
		{
			while (!Scheduled_.isEmpty ())
				Scheduled_.takeFirst () (GetNextThread ());
			return;
		}

		const auto leastBusyThread = GetNextThread ();
		if (!leastBusyThread->GetQueueSize ())
			Scheduled_.takeFirst () (leastBusyThread);

		CheckingNext_ = true;

		auto thread = CreateThread ();

		Util::Sequence (this, thread->Schedule (TaskPriority::High, &AccountThreadWorker::TestConnectivity)) >>
			[this, thread] (const auto& result)
			{
				CheckingNext_ = false;

				Util::Visit (result.AsVariant (),
						[this, thread] (Util::Void)
						{
							ExistingThreads_ << thread;
							RunScheduled (thread.get ());
						},
						[this, thread] (const auto& err)
						{
							Util::Visit (err,
									[this, thread] (const vmime::exceptions::authentication_error& err)
									{
										qWarning () << "Snails::ThreadPool::RunThreads():"
												<< "got auth error:"
												<< err.what ()
												<< "; seems like connections limit at"
												<< ExistingThreads_.size ();
										HitLimit_ = true;

										HandleThreadOverflow (thread);

										while (!Scheduled_.isEmpty ())
											Scheduled_.takeFirst () (GetNextThread ());
									},
									[] (const auto& e) { qWarning () << "Snails::ThreadPool::RunThreads():" << e.what (); });
						});
			};
	}

	AccountThread_ptr ThreadPool::CreateThread ()
	{
		const auto& threadName = "PooledThread_" + QString::number (ExistingThreads_.size ());
		const auto thread = std::make_shared<AccountThread> (false,
				threadName, Acc_, Storage_);
		thread->SetQuitWait (ULONG_MAX);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, thread]
			{
				for (const auto& init : ThreadInitializers_)
					init (thread.get ());
			},
			thread.get (),
			SIGNAL (started ()),
			thread.get ()
		};

		thread->start (QThread::LowPriority);

		return thread;
	}

	void ThreadPool::RunScheduled (AccountThread *thread)
	{
		if (!Scheduled_.isEmpty ())
			Scheduled_.takeFirst () (thread);

		if (!Scheduled_.isEmpty ())
			QTimer::singleShot (0, this, &ThreadPool::RunThreads);
	}

	AccountThread* ThreadPool::GetNextThread ()
	{
		if (ExistingThreads_.isEmpty ())
			ExistingThreads_ << CreateThread ();

		const auto min = std::min_element (ExistingThreads_.begin (), ExistingThreads_.end (),
				Util::ComparingBy ([] (const auto& ptr) { return ptr->GetQueueSize (); }));
		return min->get ();
	}

	void ThreadPool::HandleThreadOverflow (AccountThread *thread)
	{
		const auto pos = std::find_if (ExistingThreads_.begin (), ExistingThreads_.end (),
				[thread] (const auto& ptr) { return ptr.get () == thread; });
		if (pos == ExistingThreads_.end ())
		{
			qWarning () << "Snails::ThreadPool::HandleThreadOverflow():"
					<< "thread"
					<< thread
					<< "is not found among existing threads";
			return;
		}

		HandleThreadOverflow (*pos);
	}

	void ThreadPool::HandleThreadOverflow (const AccountThread_ptr& thread)
	{
		thread->Schedule (TaskPriority::Low, &AccountThreadWorker::Disconnect);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[thread] {},
			thread.get (),
			SIGNAL (finished ()),
			nullptr
		};
		thread->quit ();

		ExistingThreads_.removeOne (thread);
	}
}
}
