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

#pragma once

#include <QWizardPage>
#include <interfaces/core/icoreproxy.h>
#include "ui_bugreportpage.h"

namespace LeechCraft
{
namespace Dolozhee
{
	class BugReportPage : public QWizardPage
	{
		Q_OBJECT

		Ui::BugReportPage Ui_;
		ICoreProxy_ptr Proxy_;
	public:
		BugReportPage (ICoreProxy_ptr, QWidget* = 0);

		int nextId () const;

		QString GetTitle () const;
		QString GetText () const;
	};
}
}
