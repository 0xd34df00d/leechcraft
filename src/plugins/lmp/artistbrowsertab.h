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

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_artistbrowsertab.h"

namespace LeechCraft
{
namespace LMP
{
	class BioViewManager;
	class SimilarViewManager;

	class ArtistBrowserTab : public QWidget
						   , public ITabWidget
						   , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		const TabClassInfo TC_;
		QObject * const Plugin_;

		Ui::ArtistBrowserTab Ui_;

		BioViewManager *BioMgr_;
		SimilarViewManager *SimilarMgr_;
	public:
		ArtistBrowserTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;

		void Browse (const QString&);
	private slots:
		void on_ArtistNameEdit__returnPressed ();
	signals:
		void removeTab (QWidget*);

		void tabRecoverDataChanged ();
	};
}
}
