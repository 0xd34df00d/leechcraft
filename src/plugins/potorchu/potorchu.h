/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

#ifndef PLUGINS_POTORCHU_POTORCHU_H
#define PLUGINS_POTORCHU_POTORCHU_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ientityhandler.h>

#include "potorchuwidget.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		class Potorchu : public QObject
					, public IInfo
					, public IHaveTabs
					, public IEntityHandler
		{
			Q_OBJECT
			Q_INTERFACES (IInfo IHaveTabs IEntityHandler)
			
			TabClasses_t TabClasses_;
			QList<PotorchuWidget *> Others_;
			ICoreProxy_ptr Proxy_;
		public:
			void Init (ICoreProxy_ptr);
			void SecondInit ();
			QByteArray GetUniqueID () const;
			void Release ();
			QString GetName () const;
			QString GetInfo () const;
			QIcon GetIcon () const;
			QStringList Provides () const;
			QStringList Needs () const;
			QStringList Uses () const;
			void SetProvider (QObject*, const QString&);
			
			TabClasses_t GetTabClasses () const;
			void TabOpenRequested (const QByteArray& tabClass);
			
			EntityTestHandleResult CouldHandle (const LeechCraft::Entity& entity) const;
			void Handle (LeechCraft::Entity entity);
		signals:
			void addNewTab (const QString&, QWidget*);
			void removeTab (QWidget*);
			void changeTabName (QWidget*, const QString&);
			void changeTabIcon (QWidget*, const QIcon&);
			void changeTooltip (QWidget*, QWidget*);
			void statusBarChanged (QWidget*, const QString&);
			void raiseTab (QWidget*);
		private slots:
			void handleNeedToClose ();
		};
	}
}

#endif

