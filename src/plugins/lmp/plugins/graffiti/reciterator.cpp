/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "reciterator.h"
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilmputilproxy.h>

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	RecIterator::RecIterator (ILMPProxy_ptr proxy, QObject *parent)
	: QObject { parent }
	, LMPProxy_ { proxy }
	, StopFlag_ { false }
	{
	}

	void RecIterator::Start (const QString& path)
	{
		auto watcher = new QFutureWatcher<QList<QFileInfo>> (this);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleImplFinished ()));

		const auto& future = QtConcurrent::run ([this, path]
				{ return LMPProxy_->GetUtilProxy ()->RecIterateInfo (path, true, &StopFlag_); });
		watcher->setFuture (future);
	}

	QList<QFileInfo> RecIterator::GetResult () const
	{
		return Result_;
	}

	void RecIterator::cancel ()
	{
		StopFlag_.store (true, std::memory_order_relaxed);
	}

	void RecIterator::handleImplFinished ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QList<QFileInfo>>*> (sender ());
		Result_ = watcher->result ();
		watcher->deleteLater ();

		if (StopFlag_)
			emit canceled ();
		else
			emit finished ();
	}
}
}
}
