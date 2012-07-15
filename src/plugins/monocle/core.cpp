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

#include "core.h"
#include <QSettings>
#include <QApplication>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include "pixmapcachemanager.h"
#include "recentlyopenedmanager.h"
#include "choosebackenddialog.h"

namespace LeechCraft
{
namespace Monocle
{
	Core::Core ()
	: CacheManager_ (new PixmapCacheManager (this))
	, ROManager_ (new RecentlyOpenedManager (this))
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::AddPlugin (QObject *pluginObj)
	{
		auto plugin2 = qobject_cast<IPlugin2*> (pluginObj);
		const auto& classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Monocle.IBackendPlugin"))
			Backends_ << pluginObj;
	}

	bool Core::CanLoadDocument (const QString& path)
	{
		Q_FOREACH (auto backend, Backends_)
			if (qobject_cast<IBackendPlugin*> (backend)->CanLoadDocument (path))
				return true;

		return false;
	}

	IDocument_ptr Core::LoadDocument (const QString& path)
	{
		decltype (Backends_) loaders;
		Q_FOREACH (auto backend, Backends_)
			if (qobject_cast<IBackendPlugin*> (backend)->CanLoadDocument (path))
				loaders << backend;

		if (loaders.isEmpty ())
			return IDocument_ptr ();
		else if (loaders.size () == 1)
			return qobject_cast<IBackendPlugin*> (loaders.at (0))->LoadDocument (path);

		QList<QByteArray> ids;
		Q_FOREACH (auto backend, loaders)
			ids << qobject_cast<IInfo*> (backend)->GetUniqueID ();
		std::sort (ids.begin (), ids.end ());
		const auto& key = std::accumulate (ids.begin (), ids.end (), QByteArray (),
				[] (const QByteArray& left, const QByteArray& right)
					{ return left + '|' + right; });

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		std::shared_ptr<void> (static_cast<void*> (0),
				[&settings] (void*) { settings.endGroup (); });

		if (ids.contains (settings.value (key).toByteArray ()))
		{
			const auto& id = settings.value (key).toByteArray ();
			Q_FOREACH (auto backend, loaders)
				if (qobject_cast<IInfo*> (backend)->GetUniqueID () == id)
					return qobject_cast<IBackendPlugin*> (backend)->LoadDocument (path);
			return IDocument_ptr ();
		}

		ChooseBackendDialog dia (loaders);
		if (dia.exec () != QDialog::Accepted)
			return IDocument_ptr ();

		auto backend = dia.GetSelectedBackend ();
		if (dia.GetRememberChoice ())
			settings.setValue (key, qobject_cast<IInfo*> (backend)->GetUniqueID ());

		return qobject_cast<IBackendPlugin*> (backend)->LoadDocument (path);
	}

	PixmapCacheManager* Core::GetPixmapCacheManager () const
	{
		return CacheManager_;
	}

	RecentlyOpenedManager* Core::GetROManager () const
	{
		return ROManager_;
	}
}
}
