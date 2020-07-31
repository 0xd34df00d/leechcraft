/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Wt/WSortFilterProxyModel.h>
#include <common.h>

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	class ReadItemsFilter : public Wt::WSortFilterProxyModel
	{
		bool HideRead_ = true;

		QList<IDType_t> Prevs_;
		IDType_t CurrentId_ = static_cast<IDType_t> (-1);
	public:
		ReadItemsFilter ();

		void SetHideRead (bool);

		void SetCurrentItem (IDType_t);
		void ClearCurrentItem ();
	protected:
		bool filterAcceptRow (int, const Wt::WModelIndex&) const;
	private:
		void PullOnePrev ();
		void Invalidate ();
	};
}
}
}
