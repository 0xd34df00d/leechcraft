/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progressmodelmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <util/xpc/progressmanager.h>
#include "checker.h"

namespace LC::LMP::BrainSlugz
{
	ProgressModelManager::ProgressModelManager (QObject *parent)
	: QObject { parent }
	, Progress_ { new Util::ProgressManager { this } }
	{
	}

	IJobHolderRepresentationHandler_ptr ProgressModelManager::CreateReprHandler ()
	{
		return Progress_->CreateDefaultHandler ();
	}

	void ProgressModelManager::AddChecker (Checker *checker)
	{
		const auto initialCount = checker->GetRemainingCount ();
		const auto& label = tr ("Checking new album releases...");
		auto row = Progress_->AddRow ({
				.Name_ = label,
				.Specific_ = ProcessInfo { .Parameters_ = FromUserInitiated, .Kind_ = ProcessKind::Generic },
			},
			{ .Total_ = initialCount });
		connect (checker,
				&Checker::progress,
				this,
				[initialCount, row = std::move (row)] (int remaining) { row->SetDone (initialCount - remaining); });
	}
}
