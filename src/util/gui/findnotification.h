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

#pragma once

#include <util/utilconfig.h>
#include "pagenotification.h"

namespace Ui
{
	class FindNotification;
}

namespace LeechCraft
{
namespace Util
{
	class UTIL_API FindNotification : public PageNotification
	{
		Q_OBJECT

		Ui::FindNotification *Ui_;
	public:
		enum FindFlag
		{
			FindCaseSensitively,
			FindBackwards,
			FindWrapsAround
		};
		Q_DECLARE_FLAGS (FindFlags, FindFlag)

		FindNotification (QWidget*);
		~FindNotification ();

		void SetText (const QString&);
		QString GetText () const;

		void SetSuccessful (bool);
		void Focus ();

		FindFlags GetFlags () const;
	protected:
		virtual void handleNext (const QString&, FindFlags) = 0;
	private slots:
		void on_Pattern__textChanged (const QString&);
		void on_FindButton__released ();
		virtual void reject ();
	};
}
}
