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
#include <interfaces/lmp/ilmpplugin.h>
#include "ui_graffititab.h"

class QFileSystemModel;

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	class FilesModel;

	class GraffitiTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		ILMPProxy_ptr LMPProxy_;

		const TabClassInfo TC_;
		QObject * const Plugin_;

		Ui::GraffitiTab Ui_;

		QFileSystemModel *FSModel_;
		FilesModel *FilesModel_;
	public:
		GraffitiTab (ILMPProxy_ptr, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private slots:
		void on_DirectoryTree__activated (const QModelIndex&);
		void handleIterateFinished ();
		void handleScanFinished ();
	signals:
		void removeTab (QWidget*);
	};
}
}
}
