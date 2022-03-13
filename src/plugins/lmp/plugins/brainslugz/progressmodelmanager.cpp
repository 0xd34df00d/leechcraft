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
#include <util/xpc/util.h>
#include "checker.h"

namespace LC::LMP::BrainSlugz
{
	ProgressModelManager::ProgressModelManager (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	{
	}

	QAbstractItemModel* ProgressModelManager::GetModel () const
	{
		return Model_;
	}

	void ProgressModelManager::handleCheckStarted (Checker *checker)
	{
		if (!Row_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "seems like a check is already in progress";
			return;
		}

		InitialCount_ = checker->GetRemainingCount ();
		const auto& label = tr ("Checking new releases of %n artist(s)...", 0, InitialCount_);
		Row_ = QList<QStandardItem*>
		{
			new QStandardItem { label },
			new QStandardItem { tr ("Checking...") },
			new QStandardItem {}
		};

		Util::InitJobHolderRow (Row_);
		handleProgress (InitialCount_);

		Model_->appendRow (Row_);

		connect (checker,
				SIGNAL (progress (int)),
				this,
				SLOT (handleProgress (int)));
		connect (checker,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
	}

	void ProgressModelManager::handleProgress (int remaining)
	{
		const auto done = InitialCount_ - remaining;
		Util::SetJobHolderProgress (Row_, done, InitialCount_, tr ("%1 of %2"));
	}

	void ProgressModelManager::handleFinished ()
	{
		Model_->removeRow (0);
		Row_.clear ();
	}
}
