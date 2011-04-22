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

#ifndef INTERFACES_IHAVETABS_H
#define INTERFACES_IHAVETABS_H
#include <QMetaType>
#include <QList>
#include <QByteArray>
#include <QIcon>

namespace LeechCraft
{
	enum TabFeature
	{
		TFEmpty = 0x0,
		TFSingle = 0x01,
		TFOpenableByRequest = 0x02
	};
	
	Q_DECLARE_FLAGS (TabFeatures, TabFeature);

	struct TabClassInfo
	{
		QByteArray TabClass_;
		QString VisibleName_;
		QString Description_;
		QIcon Icon_;
		quint16 Priority_;

		TabFeatures Features_;
	};
	
	typedef QList<TabClassInfo> TabClasses_t;
};

class ITabWidget
{
public:
	virtual ~ITabWidget () {}
	
	virtual LeechCraft::TabClassInfo GetTabClassInfo () const;
	
	virtual void CloseRequested () const = 0;

	virtual QToolBar* GetToolBar () const = 0;
	
	virtual QList<QAction*> GetTabBarContextMenuActions () const
	{
		return QList<QAction*> ();
	}
	
	virtual QMap<QString, QList<QAction*> > GetWindowMenus () const
	{
		return QMap<QString, QList<QAction*> > ();
	}
	
	virtual void TabMadeCurrent ()
	{
	}
};

class IHaveTabs
{
public:
	virtual ~IHaveTabs () {}
	
	virtual LeechCraft::TabClasses_t GetTabClasses () const = 0;
	
	virtual void TabOpenRequested (const QByteArray& tabClass) = 0;
	
	virtual void addNewTab (const QString& name, QWidget *tabContents) = 0;
	virtual void removeTab (QWidget *tabContents) = 0;
	virtual void changeTabName (QWidget *tabContents, const QString& name) = 0;
	virtual void changeTabIcon (QWidget *tabContents, const QIcon& icon) = 0;
	virtual void statusBarChanged (QWidget *tabContents, const QString& text) = 0;
	virtual void raiseTab (QWidget *tabContents) = 0;
};

Q_DECLARE_INTERFACE (ITabWidget, "org.Deviant.LeechCraft.ITabWidget/1.0");
Q_DECLARE_INTERFACE (IHaveTabs, "org.Deviant.LeechCraft.IHaveTabs/1.0");

#endif
