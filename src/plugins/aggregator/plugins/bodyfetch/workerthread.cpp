/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "workerthread.h"
#include <QtDebug>
#include "workerobject.h"

namespace LeechCraft
{
namespace Aggregator
{
namespace BodyFetch
{
	WorkerThread::WorkerThread (QObject *parent)
	: QThread (parent)
	, Object_ (0)
	{
	}
	
	void WorkerThread::run ()
	{
		Object_ = new WorkerObject ();
		QThread::run ();
		Object_ = 0;
	}
	
	void WorkerThread::SetLoaderInstance (IScriptLoaderInstance *inst)
	{
		Object_->SetLoaderInstance (inst);
	}
	
	bool WorkerThread::IsOK () const
	{
		return isRunning () && Object_ && Object_->IsOk ();
	}
	
	void WorkerThread::AppendItems (const QVariantList& items)
	{
		Object_->AppendItems (items);
		QMetaObject::invokeMethod (Object_,
				"process",
				Qt::QueuedConnection);
	}
	
	QObject* WorkerThread::GetWorkingObject () const
	{
		return Object_;
	}
}
}
}
