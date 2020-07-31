/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "microblogstab.h"
#include <interfaces/azoth/iaccount.h>

namespace LC
{
namespace Azoth
{
	QObject* MicroblogsTab::S_ParentMultiTabs_ = 0;
	TabClassInfo MicroblogsTab::S_TC_ = TabClassInfo ();

	void MicroblogsTab::SetTabData (QObject *obj, const TabClassInfo& tc)
	{
		S_ParentMultiTabs_ = obj;
		S_TC_ = tc;
	}

	MicroblogsTab::MicroblogsTab (IAccount *acc)
	: Account_ (acc)
	{
		Ui_.setupUi (this);
	}

	TabClassInfo MicroblogsTab::GetTabClassInfo () const
	{
		return S_TC_;
	}

	QObject* MicroblogsTab::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	void MicroblogsTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* MicroblogsTab::GetToolBar () const
	{
		return 0;
	}
}
}
