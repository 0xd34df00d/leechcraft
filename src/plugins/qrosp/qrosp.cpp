/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "qrosp.h"
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <qross/core/manager.h>
#include <interfaces/entitytesthandleresult.h>
#include "pluginmanager.h"
#include "wrapperobject.h"
#include "scriptloaderinstance.h"

Q_DECLARE_METATYPE (QObject**);

namespace LC
{
namespace Qrosp
{
	void Plugin::Init (ICoreProxy_ptr /* TODO proxy */)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
		PluginManager::Instance ().Release ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Qrosp";
	}

	QString Plugin::GetName () const
	{
		return "Qrosp";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Makes LeechCraft scriptable using Qross.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QStringList Plugin::Provides () const
	{
		return QStringList ("qrosp");
	}

	QList<QObject*> Plugin::GetPlugins ()
	{
		return PluginManager::Instance ().GetPlugins ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		QString language = entity.Additional_ ["Language"].toString ().toLower ();
		if (entity.Mime_ != "x-leechcraft/script-wrap-request")
			return EntityTestHandleResult ();
		if (!entity.Additional_ ["Object"].value<QObject**> ())
			return EntityTestHandleResult ();
		if (!Qross::Manager::self ().interpreters ().contains (language))
			return EntityTestHandleResult ();
		if (!entity.Entity_.toUrl ().isValid ())
			return EntityTestHandleResult ();
		if (!QFileInfo (entity.Entity_
				.toUrl ().toLocalFile ()).exists ())
			return EntityTestHandleResult ();

		return EntityTestHandleResult (EntityTestHandleResult::PIdeal);
	}

	void Plugin::Handle (Entity entity)
	{
		QString language = entity.Additional_ ["Language"].toString ().toLower ();
		QString path = entity.Entity_.toUrl ().toLocalFile ();

		*entity.Additional_ ["Object"].value<QObject**> () = new WrapperObject (language, path);
	}

	IScriptLoaderInstance_ptr Plugin::CreateScriptLoaderInstance (const QString& relPath)
	{
		return std::make_shared<ScriptLoaderInstance> (relPath);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_qrosp, LC::Qrosp::Plugin);
