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

#ifndef COREPROXY_H
#define COREPROXY_H
#include <QObject>
#include "interfaces/core/icoreproxy.h"
#include "util/idpool.h"

namespace LeechCraft
{
	class MWProxy;

	/** Implements the ICoreProxy's interface.
	 */
	class CoreProxy : public QObject
					, public ICoreProxy
	{
		Q_OBJECT
		Q_INTERFACES (ICoreProxy);

		Util::IDPool<int> Pool_;
		MWProxy *MWProxy_;
	public:
		CoreProxy (QObject* = 0);
		QNetworkAccessManager* GetNetworkAccessManager () const;
		IShortcutProxy* GetShortcutProxy () const;
		IMWProxy* GetMWProxy () const;
		QModelIndex MapToSource (const QModelIndex&) const;
		Util::BaseSettingsManager* GetSettingsManager () const;
		QMainWindow* GetMainWindow () const;
		ICoreTabWidget* GetTabWidget () const;
		QIcon GetIcon (const QString&, const QString& = QString ()) const;
		void UpdateIconset (const QList<QAction*>&) const;
		ITagsManager* GetTagsManager () const;
		QStringList GetSearchCategories () const;
		int GetID ();
		void FreeID (int);
		IPluginsManager* GetPluginsManager () const;
		QString GetVersion () const;
		QObject* GetSelf ();
		void RegisterSkinnable (QAction*);
		bool IsShuttingDown ();
	};
};

#endif

