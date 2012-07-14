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

#ifndef PLUGINS_JUFFED_JUFFED_H
#define PLUGINS_JUFFED_JUFFED_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>

namespace LeechCraft
{
namespace JuffEd
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs)
		
		TabClasses_t TabClasses_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);
	signals:
		void addNewTab (const QString& name, QWidget *tabContents);
		void removeTab (QWidget *tabContents);
		void changeTabName (QWidget *tabContents, const QString& name);
		void changeTabIcon (QWidget *tabContents, const QIcon& icon);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget *tabContents, const QString& text);
		void raiseTab (QWidget *tabContents);
	};
}
}

#endif

