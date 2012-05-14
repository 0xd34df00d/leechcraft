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

#include "documenttab.h"
#include "core.h"
#include "pagegraphicsitem.h"

namespace LeechCraft
{
namespace Monocle
{
	DocumentTab::DocumentTab (const TabClassInfo& tc, QObject* parent)
	: TC_ (tc)
	, ParentPlugin_ (parent)
	{
		Ui_.setupUi (this);
		Ui_.PagesView_->setScene (&Scene_);
		Ui_.PagesView_->setBackgroundBrush (palette ().brush (QPalette::Dark));

		CurrentDoc_ = Core::Instance ().LoadDocument ("/home/d34df00d/Programming/Generating/docs/Rudoy2012Generation.pdf");
		for (int i = 0, size = CurrentDoc_->GetNumPages (); i < size; ++i)
		{
			auto item = new PageGraphicsItem (CurrentDoc_, i);
			Scene_.addItem (item);

			const auto& size = CurrentDoc_->GetPageSize (i);
			item->moveBy (0, (size.height () + 10) * i);

			Pages_ << item;
		}
	}

	TabClassInfo DocumentTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* DocumentTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void DocumentTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* DocumentTab::GetToolBar () const
	{
		return 0;
	}
}
}
