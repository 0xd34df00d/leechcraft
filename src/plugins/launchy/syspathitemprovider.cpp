/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syspathitemprovider.h"
#include <QStandardItemModel>
#include <QTimer>
#include <QtDebug>
#include <QProcess>
#include <util/sys/paths.h>
#include "modelroles.h"

namespace LC
{
namespace Launchy
{
	SysPathItemProvider::SysPathItemProvider (QStandardItemModel *model, QObject *parent)
	: QObject (parent)
	, Model_ (model)
	, SearchPathScheduled_ (false)
	, PathItem_ (new QStandardItem)
	{
		PathItem_->setData (QString (), ModelRoles::ItemIcon);
		PathItem_->setData (QStringList ("X-Console"), ModelRoles::ItemNativeCategories);
		PathItem_->setData (false, ModelRoles::IsItemFavorite);

		auto executor = [this]
		{
			const auto& cmd = PathItem_->data (ModelRoles::ItemID).toString ();
			if (!cmd.isEmpty ())
				QProcess::startDetached (cmd, {});
		};
		PathItem_->setData (QVariant::fromValue<Executor_f> (executor),
				ModelRoles::ExecutorFunctor);
	}

	void SysPathItemProvider::HandleQuery (const QString& query)
	{
		CurrentQuery_ = query;
		ScheduleSearch ();
	}

	void SysPathItemProvider::ScheduleSearch ()
	{
		if (SearchPathScheduled_)
			return;

		SearchPathScheduled_ = true;
		QTimer::singleShot (200,
				this,
				SLOT (searchPath ()));
	}

	void SysPathItemProvider::searchPath ()
	{
		SearchPathScheduled_ = false;

		const auto& candidate = Util::FindInSystemPath (CurrentQuery_,
				Util::GetSystemPaths (),
				[] (const QFileInfo& fi) { return fi.isExecutable () && fi.isFile (); });

		if (candidate.isEmpty ())
		{
			if (PathItem_->row () != -1)
				Model_->takeRow (PathItem_->row ());
			return;
		}

		PathItem_->setData (CurrentQuery_, ModelRoles::ItemName);
		PathItem_->setData (CurrentQuery_, ModelRoles::ItemDescription);
		PathItem_->setData (candidate, ModelRoles::ItemID);
		if (PathItem_->row () == -1)
			Model_->appendRow (PathItem_);
	}
}
}
