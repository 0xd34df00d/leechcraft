/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "reciterator.h"
#include <QtConcurrentRun>
#include <util/threads/futures.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilmputilproxy.h>

namespace LC::LMP::Graffiti
{
	RecIterator::RecIterator (ILMPProxy_ptr proxy, QObject *parent)
	: QObject { parent }
	, LMPProxy_ { proxy }
	{
	}

	void RecIterator::Start (const QString& path)
	{
		const auto& future = QtConcurrent::run ([this, path]
				{ return LMPProxy_->GetUtilProxy ()->RecIterateInfo (path, true, &StopFlag_); });

		Util::Sequence (this, future) >>
				[this] (const QList<QFileInfo>& files)
				{
					Result_ = files;

					if (StopFlag_)
						emit canceled ();
					else
						emit finished ();
				};
	}

	QList<QFileInfo> RecIterator::GetResult () const
	{
		return Result_;
	}

	void RecIterator::Cancel ()
	{
		StopFlag_.store (true, std::memory_order_relaxed);
	}
}
