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

#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H
#include <QWidget>
#include "interfaces/ihavetabs.h"
#include "ui_settingstab.h"

class IHaveSettings;
class QLineEdit;
class QToolButton;

namespace LeechCraft
{
	class SettingsTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::SettingsTab Ui_;
		QToolBar *Toolbar_;
		QAction *ActionBack_;
		QAction *ActionApply_;
		QAction *ActionCancel_;

		QString LastSearch_;
		QHash<QToolButton*, QObject*> Button2SettableRoot_;
		QHash<IHaveSettings*, QList<int>> Obj2SearchMatchingPages_;
		QHash<QTreeWidgetItem*, QPair<IHaveSettings*, int>> Item2Page_;
	public:
		SettingsTab (QWidget* = 0);

		void Initialize ();

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void FillPages (QObject*, bool);
	public slots:
		void showSettingsFor (QObject*);
	private slots:
		void addSearchBox ();
		void handleSearch (const QString&);
		void handleSettingsCalled ();
		void handleSettingsForObject ();
		void handleBackRequested ();
		void handleApply ();
		void handleCancel ();
		void on_Cats__currentItemChanged (QTreeWidgetItem*);
	signals:
		void remove (QWidget*);
	};
}

#endif
