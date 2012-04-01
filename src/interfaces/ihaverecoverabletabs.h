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

#ifndef INTERFACES_IHAVERECOVERABLETABS_H
#define INTERFACES_IHAVERECOVERABLETABS_H
#include <QList>
#include <QByteArray>
#include <QVariant>

class QWidget;
class QIcon;

class IRecoverableTab
{
public:
	virtual ~IRecoverableTab () {}

	virtual QByteArray GetTabRecoverData () const = 0;
	virtual QString GetTabRecoverName () const = 0;
	virtual QIcon GetTabRecoverIcon () const = 0;
protected:
	virtual void tabRecoverDataChanged () = 0;
};

namespace LeechCraft
{
	typedef QList<QPair<QByteArray, QVariant>> DynPropertiesList_t;
	struct TabRecoverInfo
	{
		QByteArray Data_;
		DynPropertiesList_t DynProperties_;
	};
}

class IHaveRecoverableTabs
{
public:
	virtual ~IHaveRecoverableTabs () {}

	virtual void RecoverTabs (const QList<LeechCraft::TabRecoverInfo>&) = 0;
};

Q_DECLARE_INTERFACE (IRecoverableTab, "org.Deviant.LeechCraft.IRecoverableTab/1.0");
Q_DECLARE_INTERFACE (IHaveRecoverableTabs, "org.Deviant.LeechCraft.IHaveRecoverableTabs/1.0");

#endif
