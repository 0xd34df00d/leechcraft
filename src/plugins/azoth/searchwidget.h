/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_SEARCHWIDGET_H
#define PLUGINS_AZOTH_SEARCHWIDGET_H
#include <boost/shared_ptr.hpp>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_searchwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	class IHaveSearch;
	class ISearchSession;

	class SearchWidget : public QWidget
					   , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget);

		static QObject *S_ParentMultiTabs_;

		Ui::SearchWidget Ui_;
		boost::shared_ptr<ISearchSession> CurrentSess_;
	public:
		static void SetParentMultiTabs (QObject*);

		SearchWidget (QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		IHaveSearch* GetCurrentSearch () const;
	private slots:
		void search ();
		void on_AccountBox__activated (int);
	signals:
		void removeTab (QWidget*);
	};
}
}

#endif
