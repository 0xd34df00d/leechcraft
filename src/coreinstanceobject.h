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

#ifndef COREINSTANCEOBJECT_H
#define COREINSTANCEOBJECT_H
#include <QObject>
#include "interfaces/iinfo.h"
#include "interfaces/ihavesettings.h"
#include "interfaces/ihavetabs.h"

namespace LeechCraft
{
	class SettingsTab;

	class CoreInstanceObject : public QObject
							 , public IInfo
							 , public IHaveSettings
							 , public IHaveTabs
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IHaveTabs)
		
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		TabClasses_t Classes_;
		
		SettingsTab *SettingsTab_;
	public:
		CoreInstanceObject (QObject* = 0);
		
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
		
		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);
	private:
		void BuildNewTabModel ();
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}

#endif
