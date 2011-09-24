/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
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

#include "choroidtab.h"

namespace LeechCraft
{
namespace Choroid
{
	ChoroidTab::ChoroidTab (const TabClassInfo& tc, QObject *parent)
	: TabClass_ (tc)
	, Parent_ (parent)
	{
		Ui_.setupUi (this);
	}

	TabClassInfo ChoroidTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* ChoroidTab::ParentMultiTabs ()
	{
		return Parent_;
	}

	void ChoroidTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ChoroidTab::GetToolBar () const
	{
		return 0;
	}
}
}
