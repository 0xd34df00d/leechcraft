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

#ifndef PLUGINS_HISTORYHOLDER_HISTORYHOLDER_H
#define PLUGINS_HISTORYHOLDER_HISTORYHOLDER_H
#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihaveshortcuts.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace HistoryHolder
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IFinder
						 , public IEntityHandler
						 , public IHaveShortcuts
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IFinder IEntityHandler IHaveShortcuts)
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				QStringList GetCategories () const;
				QList<IFindProxy_ptr> GetProxy (const LeechCraft::Request&);

				bool CouldHandle (const LeechCraft::Entity&) const;
				void Handle (LeechCraft::Entity);

				void SetShortcut (const QString&, const QKeySequences_t&);
				QMap<QString, ActionInfo> GetActionInfo () const;
			public slots:
				void handleTasksTreeActivated (const QModelIndex&);
			signals:
				void gotEntity (const LeechCraft::Entity&);
				void categoriesChanged (const QStringList&, const QStringList&);
			};
		};
	};
};

#endif

