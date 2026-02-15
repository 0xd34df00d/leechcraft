/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progressmanager.h"
#include <QStandardItemModel>
#include <util/xpc/progressmanager.h>
#include <interfaces/ijobholder.h>
#include "cuesplitter.h"

namespace LC::LMP::Graffiti
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject { parent }
	, Manager_ { new Util::ProgressManager { this } }
	{
	}

	QAbstractItemModel* ProgressManager::GetModel () const
	{
		return &Manager_->GetModel ();
	}

	void ProgressManager::HandleTagsFetch (int fetched, int total, QObject *obj)
	{
		if (fetched == total)
		{
			TagsFetchObj2Row_.remove (obj);
			return;
		}

		auto& row = TagsFetchObj2Row_ [obj];
		if (!row)
			row = Manager_->AddRow ({
						.Name_ = tr ("Fetching tags...."),
						.Specific_ = ProcessInfo { .Kind_ = ProcessKind::Generic }
					},
					{ .Total_ = total });

		row->SetDone (fetched);
		row->SetTotal (total);
	}

	void ProgressManager::HandleCueSplitter (CueSplitter *splitter)
	{
		auto row = Manager_->AddRow ({
					.Name_ = tr ("Splitting CUE %1...").arg (splitter->GetCueFile ()),
					.Specific_ = ProcessInfo { .Kind_ = ProcessKind::Generic },
				});
		connect (splitter,
				&CueSplitter::splitProgress,
				this,
				[row = std::move (row)] (int done, int total)
				{
					row->SetDone (done);
					row->SetTotal (total);
				});
	}
}
