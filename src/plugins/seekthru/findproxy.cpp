/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "findproxy.h"
#include <util/models/mergemodel.h>

namespace LC::SeekThru
{
	FindProxy::FindProxy (Request r)
	: R_ (std::move (r))
	, MergeModel_ (new Util::MergeModel ({ "1", "2", "3" }))
	{
	}

	FindProxy::~FindProxy ()
	{
		for (const auto& sh : Handlers_)
			MergeModel_->RemoveModel (sh.get ());
	}

	QAbstractItemModel* FindProxy::GetModel ()
	{
		return MergeModel_.get ();
	}

	QByteArray FindProxy::GetUniqueSearchID () const
	{
		return QString ("org.LeechCraft.SeekThru.%1.%2")
				.arg (R_.Category_)
				.arg (R_.String_)
				.toUtf8 ();
	}

	QStringList FindProxy::GetCategories () const
	{
		return { R_.Category_ };
	}

	void FindProxy::SetHandlers (const QList<SearchHandler_ptr>& handlers)
	{
		Handlers_ = handlers;
		for (const auto& sh : Handlers_)
		{
			MergeModel_->AddModel (sh.get ());
			sh->Start (R_);
		}
	}
}
