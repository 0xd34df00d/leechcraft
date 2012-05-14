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

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "interfaces/monocle/idocument.h"
#include "ui_documenttab.h"

namespace LeechCraft
{
namespace Monocle
{
	class DocumentTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::DocumentTab Ui_;

		TabClassInfo TC_;
		QObject *ParentPlugin_;

		IDocument_ptr CurrentDoc_;
	public:
		DocumentTab (const TabClassInfo&, QObject*);

		virtual TabClassInfo GetTabClassInfo () const;
		virtual QObject* ParentMultiTabs ();
		virtual void Remove ();
		virtual QToolBar* GetToolBar () const;
	signals:
		void removeTab (QWidget*);
	};
}
}
