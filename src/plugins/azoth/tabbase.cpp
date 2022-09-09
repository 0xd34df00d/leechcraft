/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabbase.h"

namespace LC::Azoth
{
	QObject* TabBase::S_ParentMultiTabs_ = nullptr;
	TabClassInfo TabBase::S_TC_ = {};

	void TabBase::SetTabData (QObject *plugin, const TabClassInfo& tc)
	{
		S_ParentMultiTabs_ = plugin;
		S_TC_ = tc;
	}

	TabClassInfo TabBase::GetTabClassInfo () const
	{
		return S_TC_;
	}

	QObject* TabBase::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}
}
