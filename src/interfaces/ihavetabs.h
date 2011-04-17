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

#ifndef INTERFACES_IHAVESHORTCUTS_H
#define INTERFACES_IHAVESHORTCUTS_H
#include <QMetaType>
#include <QList>
#include <QByteArray>

namespace LeechCraft
{
	enum TabFeature
	{
		TFSingle = 0x01,
		TFOpenableByRequest = 0x02
	};
	
	Q_DECLARE_FLAGS (TabFeatures, TabFeature);

	struct TabClassInfo
	{
		QByteArray TabClass_;
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
};

class IHaveTabs
{
public:
	virtual ~IHaveTabs () {}
	
	virtual LeechCraft::TabClasses_t GetTabClasses () const = 0;
	
	virtual void TabOpenRequested (const QByteArray& tabClass) = 0;
};

Q_DECLARE_INTERFACE (ITabWidget, "org.Deviant.LeechCraft.ITabWidget/1.0");
Q_DECLARE_INTERFACE (IHaveTabs, "org.Deviant.LeechCraft.IHaveTabs/1.0");

#endif
