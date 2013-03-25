/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "finddialog.h"

namespace LeechCraft
{
namespace Poshuku
{
	FindDialog::FindDialog (QWidget *parent)
	: Util::FindNotification (parent)
	{
	}

	namespace
	{
		QWebPage::FindFlags ToPageFlags (Util::FindNotification::FindFlags flags)
		{
			QWebPage::FindFlags pageFlags;
			auto check = [&pageFlags, flags] (Util::FindNotification::FindFlag ourFlag, QWebPage::FindFlag pageFlag)
			{
				if (flags & ourFlag)
					pageFlags |= pageFlag;
			};
			check (Util::FindNotification::FindCaseSensitively, QWebPage::FindCaseSensitively);
			check (Util::FindNotification::FindBackwards, QWebPage::FindBackward);
			check (Util::FindNotification::FindWrapsAround, QWebPage::FindWrapsAroundDocument);
			return pageFlags;
		}
	}

	QWebPage::FindFlags FindDialog::GetPageFlags () const
	{
		return ToPageFlags (GetFlags ());
	}

	void FindDialog::handleNext (const QString& text, FindFlags flags)
	{
		emit next (text, ToPageFlags (flags));
	}
}
}
