/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "microblogstab.h"
#include <interfaces/azoth/iaccount.h>

namespace LeechCraft
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
