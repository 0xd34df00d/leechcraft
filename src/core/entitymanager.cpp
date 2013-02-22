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

#include "entitymanager.h"
#include <functional>
#include <algorithm>
#include <QDesktopServices>
#include <QUrl>
#include "util/util.h"
#include "interfaces/structures.h"
#include "interfaces/idownload.h"
#include "interfaces/ientityhandler.h"
#include "interfaces/entitytesthandleresult.h"
#include "core.h"
#include "pluginmanager.h"
#include "xmlsettingsmanager.h"
#include "handlerchoicedialog.h"

namespace LeechCraft
{
	EntityManager::EntityManager (QObject *parent)
	: QObject (parent)
	{
	}

	namespace
	{
		template<typename T>
		QObjectList GetSubtype (const Entity& e, bool fullScan,
				std::function<EntityTestHandleResult (Entity, T)> queryFunc)
		{
			auto pm = Core::Instance ().GetPluginManager ();
			QObjectList result;
			for (const auto& plugin : pm->GetAllCastableRoots<T> ())
			{
				EntityTestHandleResult r;
				try
				{
					r = queryFunc (e, qobject_cast<T> (plugin));
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< "could not query"
						<< e.what ()
						<< plugin;
					continue;
				}
				catch (...)
				{
					qWarning () << Q_FUNC_INFO
						<< "could not query"
						<< plugin;
					continue;
				}
				if (r.HandlePriority_ <= 0)
					continue;

				if (r.CancelOthers_)
					result.clear ();

				result << plugin;

				if (r.CancelOthers_ || (!fullScan && !result.isEmpty ()))
					break;
			}
			return result;
		}

		QObjectList GetObjects (const Entity& e, int *downloaders = 0, int *handlers = 0)
		{
			const auto& unwanted = e.Additional_ ["IgnorePlugins"].toStringList ();
			auto removeUnwanted = [&unwanted] (QObjectList& handlers)
			{
				const auto remBegin = std::remove_if (handlers.begin (), handlers.end (),
						[&unwanted] (QObject *obj)
							{ return unwanted.contains (qobject_cast<IInfo*> (obj)->GetUniqueID ()); });
				handlers.erase (remBegin, handlers.end ());
			};

			QObjectList result;
			if (!(e.Parameters_ & TaskParameter::OnlyHandle))
			{
				auto sub = GetSubtype<IDownload*> (e, true,
						[] (Entity e, IDownload *dl) { return dl->CouldDownload (e); });
				removeUnwanted (sub);
				if (downloaders)
					*downloaders = sub.size ();
				result += sub;
			}
			if (!(e.Parameters_ & TaskParameter::OnlyDownload))
			{
				auto sub = GetSubtype<IEntityHandler*> (e, true,
						[] (Entity e, IEntityHandler *eh) { return eh->CouldHandle (e); });
				removeUnwanted (sub);
				if (handlers)
					*handlers = sub.size ();
				result += sub;
			}
			return result;
		}

		bool NoHandlersAvailable (const Entity& e)
		{
			const auto& url = e.Entity_.toUrl ();
			if (url.isValid () &&
					(e.Parameters_ & FromUserInitiated) &&
					!(e.Parameters_ & OnlyDownload) &&
					XmlSettingsManager::Instance ()->
						property ("FallbackExternalHandlers").toBool ())
			{
				QDesktopServices::openUrl (url);
				return true;
			}
			else
				return false;
		}

		bool GetPreparedObjectList (Entity& e, QObject *desired, QObjectList& handlers, bool handling)
		{
			int numDownloaders = 0, numHandlers = 0;
			if (desired)
				handlers << desired;
			else
				handlers = GetObjects (e, &numDownloaders, &numHandlers);

			if (handlers.isEmpty () && !desired)
				return handling ? NoHandlersAvailable (e) : false;

			bool shouldAsk = false;
			if (e.Parameters_ & FromUserInitiated && !(e.Parameters_ & AutoAccept))
			{
				const bool askHandlers = XmlSettingsManager::Instance ()->property ("DontAskWhenSingle").toBool () ?
						numHandlers > 1 :
						numHandlers;
				const bool askDownloaders = (e.Parameters_ & IsDownloaded) ?
						numDownloaders > 1 :
						numDownloaders;
				shouldAsk = askHandlers || askDownloaders || (numHandlers && numDownloaders);
			}

			if (shouldAsk)
			{
				HandlerChoiceDialog dia (Util::GetUserText (e));
				for (auto handler : handlers.mid (0, numDownloaders))
					dia.Add (qobject_cast<IInfo*> (handler), qobject_cast<IDownload*> (handler));
				for (auto handler : handlers.mid (numDownloaders, numHandlers))
					dia.Add (qobject_cast<IInfo*> (handler), qobject_cast<IEntityHandler*> (handler));
				dia.SetFilenameSuggestion (e.Location_);
				if (dia.exec () != QDialog::Accepted || !dia.GetSelected ())
					return false;

				auto selected = dia.GetSelected ();
				if (qobject_cast<IDownload*> (selected))
				{
					const QString& dir = dia.GetFilename ();
					if (dir.isEmpty ())
						return false;
					e.Location_ = dir;
				}

				handlers.clear ();
				handlers << selected;
			}

			return true;
		}
	}

	IEntityManager::DelegationResult EntityManager::DelegateEntity (Entity e, QObject *desired)
	{
		e.Parameters_ |= OnlyDownload;
		QObjectList handlers;
		const bool foundOk = GetPreparedObjectList (e, desired, handlers, false);
		if (!foundOk)
			return { 0, 0 };

		for (auto obj : handlers)
			if (auto idl = qobject_cast<IDownload*> (obj))
				return { obj, idl->AddJob (e) };

		return { 0, 0 };
	}

	bool EntityManager::CouldHandle (const Entity& e)
	{
		return !GetObjects (e).isEmpty ();
	}

	bool EntityManager::HandleEntity (Entity e, QObject *desired)
	{
		QObjectList handlers;
		const bool foundOk = GetPreparedObjectList (e, desired, handlers, true);
		if (!foundOk || handlers.isEmpty ())
			return false;
		if (foundOk && handlers.isEmpty ())
			return true;

		if (!(e.Parameters_ & TaskParameter::OnlyHandle))
			for (auto obj : handlers)
				if (auto idl = qobject_cast<IDownload*> (obj))
				{
					idl->AddJob (e);
					return true;
				}

		if (!(e.Parameters_ & TaskParameter::OnlyDownload))
		{
			for (auto obj : handlers)
				if (auto ieh = qobject_cast<IEntityHandler*> (obj))
					ieh->Handle (e);
			return !handlers.isEmpty ();
		}

		return false;
	}

	QList<QObject*> EntityManager::GetPossibleHandlers (const Entity& e)
	{
		return GetObjects (e);
	}
}
