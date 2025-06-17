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
	MicroblogsTab::MicroblogsTab (IAccount*)
	{
		Ui_.setupUi (this);
	}

	void MicroblogsTab::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* MicroblogsTab::GetToolBar () const
	{
		return nullptr;
	}
}
}
