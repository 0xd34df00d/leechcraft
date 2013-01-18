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

#include <QObject>
#include <QHash>
#include <libmtp.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/lmp/ilmpplugin.h>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveTabs
				 , public ILMPPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveTabs
				LeechCraft::LMP::ILMPPlugin)

		ILMPProxy_ptr LMPProxy_;

		TabClassInfo TaggerTC_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray& tabClass);

		void SetLMPProxy (ILMPProxy_ptr);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void raiseTab (QWidget*);
		void statusBarChanged (QWidget*, const QString&);
	};
}
}
}
