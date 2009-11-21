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

#ifndef COREPROXY_H
#define COREPROXY_H
#include <QObject>
#include "interfaces/iinfo.h"
#include "plugininterface/guarded.h"

namespace LeechCraft
{
	/** Implements the ICoreProxy's interface.
	 */
	class CoreProxy : public QObject
					, public ICoreProxy
	{
		Q_OBJECT
		Q_INTERFACES (ICoreProxy);

		Util::Guarded<QList<int> > UsedIDs_;
	public:
		CoreProxy (QObject* = 0);
		QNetworkAccessManager* GetNetworkAccessManager () const;
		const IShortcutProxy* GetShortcutProxy () const;
		QTreeView* GetCurrentView () const;
		QModelIndex MapToSource (const QModelIndex&) const;
		Util::BaseSettingsManager* GetSettingsManager () const;
		QMainWindow* GetMainWindow () const;
		QTabWidget* GetTabWidget () const;
		QIcon GetIcon (const QString&, const QString& = QString ()) const;
		ITagsManager* GetTagsManager () const;
		QStringList GetSearchCategories () const;
		int GetID ();
		void FreeID (int);
		QObject* GetTreeViewReemitter () const;
		IPluginsManager* GetPluginsManager () const;
		QObject* GetSelf ();

#define LC_DEFINE_REGISTER(a) void RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t);
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
		LC_EXPANDER (HOOKS_TYPES_LIST);
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
	};
};

#endif

