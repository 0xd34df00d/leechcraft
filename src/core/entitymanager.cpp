/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entitymanager.h"
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
#include "interfaces/an/entityfields.h"
#include "core.h"
#include "pluginmanager.h"
#include "xmlsettingsmanager.h"
#include "handlerchoicedialog.h"

namespace LC
{
	EntityManager::EntityManager (QObject *plugin, QObject *parent)
	: QObject { parent }
	, Plugin_ { plugin }
	{
	}

	namespace
	{
		template<typename T, typename F>
		QObjectList GetSubtype (const Entity& e, const F& queryFunc)
		{
			auto pm = Core::Instance ().GetPluginManager ();
			QMap<int, QObjectList> result;
			for (const auto& plugin : pm->GetAllCastableRoots<T> ())
			{
				EntityTestHandleResult r;
				try
				{
					r = queryFunc (e, qobject_cast<T> (plugin));
					if (r.HandlePriority_ > 0)
						result [r.HandlePriority_] << plugin;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< "could not query"
						<< e.what ()
						<< plugin;
				}
				catch (...)
				{
					qWarning () << Q_FUNC_INFO
						<< "could not query"
						<< plugin;
				}
			}

			if (result.isEmpty ())
				return {};
			return result.last ();
		}

		QObjectList GetObjects (QObject *self, const Entity& e, int *downloaders = 0, int *handlers = 0)
		{
			if (Core::Instance ().IsShuttingDown ())
				return {};

			auto removeUnwanted = [&] (QObjectList& handlers)
			{
				if (e.Additional_ [IgnoreSelf].toBool ())
					handlers.removeOne (self);
			};

			QObjectList result;
			if (!(e.Parameters_ & TaskParameter::OnlyHandle))
			{
				auto sub = GetSubtype<IDownload*> (e,
						[] (const Entity& e, IDownload *dl) { return dl->CouldDownload (e); });
				removeUnwanted (sub);
				if (downloaders)
					*downloaders = sub.size ();
				result += sub;
			}
			if (!(e.Parameters_ & TaskParameter::OnlyDownload))
			{
				auto sub = GetSubtype<IEntityHandler*> (e,
						[] (const Entity& e, IEntityHandler *eh) { return eh->CouldHandle (e); });
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


		bool GetPreparedObjectList (QObject *self, Entity& e, QObject *desired, QObjectList& handlers, bool handling)
		{
			int numDownloaders = 0, numHandlers = 0;
			if (desired)
				handlers << desired;
			else
				handlers = GetObjects (self, e, &numDownloaders, &numHandlers);

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
	}

	IEntityManager::DelegationResult EntityManager::DelegateEntity (Entity e, QObject *desired)
	{
		if (!CheckInitStage (e, desired, DelegateAfterInit_))
			return {};

		e.Parameters_ |= OnlyDownload;
		QObjectList handlers;
		const bool foundOk = GetPreparedObjectList (Plugin_, e, desired, handlers, false);
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
					[=, this] { return CouldHandle (e); },
					Qt::BlockingQueuedConnection,
					&res);
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

		return !GetObjects (Plugin_, e).isEmpty ();
	}

	bool EntityManager::HandleEntity (Entity e, QObject *desired)
	{
		if (QThread::currentThread () != thread ())
		{
			bool res = false;
			QMetaObject::invokeMethod (this,
					[=, this] { return HandleEntity (e, desired); },
					Qt::BlockingQueuedConnection,
					&res);
			return res;
		}

		if (!CheckInitStage (e, desired, HandleAfterInit_))
			return false;

		QObjectList handlers;
		const bool foundOk = GetPreparedObjectList (Plugin_, e, desired, handlers, true);
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

		return GetObjects (Plugin_, e);
	}

	namespace
	{
		constexpr auto AllowedStage = PluginManager::InitStage::BeforeSecond;
	}

	bool EntityManager::CheckInitStage (const Entity& e, QObject *desired, QVector<QueueEntry>& queue)
	{
		const auto pm = Core::Instance ().GetPluginManager ();
		if (pm->GetInitStage () >= AllowedStage)
			return true;

		qWarning () << Q_FUNC_INFO
				<< "got an entity handle request before first init is complete:"
				<< e.Entity_;
		qWarning () << e.Additional_;
		queue << QueueEntry { e, desired };

		if (DelegateAfterInit_.isEmpty () && HandleAfterInit_.isEmpty ())
			connect (pm,
					&PluginManager::initStageChanged,
					this,
					&EntityManager::RunQueues);

		return false;
	}

	void EntityManager::RunQueues ()
	{
		const auto pm = Core::Instance ().GetPluginManager ();
		if (pm->GetInitStage () != AllowedStage)
			return;

		disconnect (pm,
				&PluginManager::initStageChanged,
				this,
				&EntityManager::RunQueues);

		for (const auto& item : DelegateAfterInit_)
			DelegateEntity (item.Entity_, item.Desired_);
		for (const auto& item : HandleAfterInit_)
			HandleEntity (item.Entity_, item.Desired_);
		DelegateAfterInit_.clear ();
		HandleAfterInit_.clear ();
	}
}
