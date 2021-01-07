/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entitymanager.h"
#include <functional>
#include <algorithm>
#include <QThread>
#include <QFuture>
#include <QDesktopServices>
#include <QUrl>
#include <QTextCodec>
#include "util/util.h"
#include "util/sll/slotclosure.h"
#include "interfaces/structures.h"
#include "interfaces/idownload.h"
#include "interfaces/ientityhandler.h"
#include "interfaces/entitytesthandleresult.h"
#include "core.h"
#include "pluginmanager.h"
#include "xmlsettingsmanager.h"
#include "handlerchoicedialog.h"

namespace LC
{
	EntityManager::EntityManager (QObject *parent)
	: QObject (parent)
	{
	}

	namespace
	{
		template<typename T, typename F>
		QObjectList GetSubtype (const Entity& e, bool fullScan, const F& queryFunc)
		{
			auto pm = Core::Instance ().GetPluginManager ();
			QMap<int, QObjectList> result;
			int cutoffPriority = 0;
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
					cutoffPriority = std::max (cutoffPriority, r.HandlePriority_);

				result [r.HandlePriority_] << plugin;

				if (!fullScan)
					break;
			}

			if (cutoffPriority > 0)
				while (!result.isEmpty ())
				{
					if (result.begin ().key () < cutoffPriority)
						result.erase (result.begin ());
					else
						break;
				}

			if (result.isEmpty ())
				return {};
			return result.last ();
		}

		QObjectList GetObjects (const Entity& e, int *downloaders = 0, int *handlers = 0)
		{
			if (Core::Instance ().IsShuttingDown ())
				return {};

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
			if (!(e.Parameters_ & FromUserInitiated) ||
					(e.Parameters_ & OnlyDownload))
				return false;

			if (!XmlSettingsManager::Instance ()->
						property ("FallbackExternalHandlers").toBool ())
				return false;

			const auto& url = e.Entity_.toUrl ();
			if (url.scheme ().isEmpty ())
				return false;

			if (e.Parameters_ & FromCommandLine)
			{
				qWarning () << Q_FUNC_INFO
						<< "refusing to pass again "
						<< url
						<< "to the environment as it was added from there";
				return false;
			}

			if (e.Parameters_ & IsDownloaded)
			{
				qDebug () << Q_FUNC_INFO
						<< "avoiding opening"
						<< url
						<< "by external apps as it is just downloaded from teh internets";
				return false;
			}

			QDesktopServices::openUrl (url);
			return true;
		}

		QString GetUserText (const Entity& p)
		{
			QString string = QObject::tr ("Too long to show");
			if (p.Additional_.contains ("UserVisibleName") &&
					p.Additional_ ["UserVisibleName"].canConvert<QString> ())
				string = p.Additional_ ["UserVisibleName"].toString ();
			else if (p.Entity_.canConvert<QByteArray> ())
			{
				QByteArray entity = p.Entity_.toByteArray ();
				if (entity.size () < 100)
					string = QTextCodec::codecForName ("UTF-8")->toUnicode (entity);
			}
			else if (p.Entity_.canConvert<QUrl> ())
			{
				string = p.Entity_.toUrl ().toString ();
				if (string.size () > 100)
					string = string.left (97) + "...";
			}
			else
				string = QObject::tr ("Binary entity");

			if (!p.Mime_.isEmpty ())
				string += QObject::tr ("<br /><br />of type <code>%1</code>").arg (p.Mime_);

			if (!p.Additional_ ["SourceURL"].toUrl ().isEmpty ())
			{
				QString urlStr = p.Additional_ ["SourceURL"].toUrl ().toString ();
				if (urlStr.size () > 63)
					urlStr = urlStr.left (60) + "...";
				string += QObject::tr ("<br />from %1")
						.arg (urlStr);
			}

			return string;
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
				HandlerChoiceDialog dia (GetUserText (e));
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

		template<typename F>
		bool CheckInitStage (const Entity& e, QObject *desired, F cont)
		{
			const auto pm = Core::Instance ().GetPluginManager ();
			if (pm->GetInitStage () != PluginManager::InitStage::BeforeFirst)
				return true;

			qWarning () << Q_FUNC_INFO
					<< "got an entity handle request before first init is complete:"
					<< e.Entity_;
			qWarning () << e.Additional_;
			new Util::SlotClosure<Util::ChoiceDeletePolicy>
			{
				[=]
				{
					if (pm->GetInitStage () == PluginManager::InitStage::BeforeFirst)
						return Util::ChoiceDeletePolicy::Delete::No;

					(EntityManager {}.*cont) (e, desired);
					return Util::ChoiceDeletePolicy::Delete::Yes;
				},
				pm,
				SIGNAL (initStageChanged (PluginManager::InitStage)),
				pm
			};
			return false;
		}
	}

	IEntityManager::DelegationResult EntityManager::DelegateEntity (Entity e, QObject *desired)
	{
		if (!CheckInitStage (e, desired, &EntityManager::DelegateEntity))
			return {};

		e.Parameters_ |= OnlyDownload;
		QObjectList handlers;
		const bool foundOk = GetPreparedObjectList (e, desired, handlers, false);
		if (!foundOk)
			return {};

		for (auto obj : handlers)
			if (auto idl = qobject_cast<IDownload*> (obj))
				return { obj, idl->AddJob (e) };

		return {};
	}

	bool EntityManager::CouldHandle (const Entity& e)
	{
		if (QThread::currentThread () != thread ())
		{
			bool res = false;
			QMetaObject::invokeMethod (this,
					"CouldHandle",
					Qt::BlockingQueuedConnection,
					Q_RETURN_ARG (bool, res),
					Q_ARG (Entity, e));
			return res;
		}

		const auto pm = Core::Instance ().GetPluginManager ();
		if (pm->GetInitStage () == PluginManager::InitStage::BeforeFirst)
		{
			qWarning () << Q_FUNC_INFO
					<< "called before first initialization stage is complete with"
					<< e.Entity_;
			qWarning () << e.Additional_;
			return false;
		}

		return !GetObjects (e).isEmpty ();
	}

	bool EntityManager::HandleEntity (Entity e, QObject *desired)
	{
		if (QThread::currentThread () != thread ())
		{
			bool res = false;
			QMetaObject::invokeMethod (this,
					"HandleEntity",
					Qt::BlockingQueuedConnection,
					Q_RETURN_ARG (bool, res),
					Q_ARG (Entity, e),
					Q_ARG (QObject*, desired));
			return res;
		}

		if (!CheckInitStage (e, desired, &EntityManager::HandleEntity))
			return false;

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
		const auto pm = Core::Instance ().GetPluginManager ();
		if (pm->GetInitStage () == PluginManager::InitStage::BeforeFirst)
		{
			qWarning () << Q_FUNC_INFO
					<< "called before first initialization stage is complete with"
					<< e.Entity_;
			qWarning () << e.Additional_;
			return {};
		}

		return GetObjects (e);
	}
}
