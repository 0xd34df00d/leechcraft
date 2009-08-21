/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef TABCONTENTSMANAGER_H
#define TABCONTENTSMANAGER_H
#include <QObject>
#include <QIcon>
#include <QList>

namespace LeechCraft
{
	class TabContents;
	class ViewReemitter;

	class TabContentsManager : public QObject
	{
		Q_OBJECT

		TabContents *Default_;
		TabContents *Current_;
		QList<TabContents*> Others_;

		ViewReemitter *Reemitter_;

		TabContentsManager ();
	public:
		static TabContentsManager& Instance ();

		void SetDefault (TabContents*);
		QList<TabContents*> GetTabs () const;

		void AddNewTab (const QString& = QString ());
		void RemoveTab (TabContents*);
		void MadeCurrent (TabContents*);
		TabContents* GetCurrent () const;
		QObject* GetReemitter () const;
	private:
		void Connect (TabContents*);
	private slots:
		void handleFilterUpdated ();
		void handleQueryUpdated (const QString&);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
};

#endif

