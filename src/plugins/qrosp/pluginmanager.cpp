/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pluginmanager.h"
#include <QDir>
#include <QModelIndex>
#include <QtDebug>
#include <qross/core/manager.h>
#include <qross/core/interpreter.h>
#include <util/sll/qtutil.h>
#include "wrapperobject.h"
#include "typesfactory.h"
#include "utilproxy.h"
#include "wrappers/entitywrapper.h"

namespace LC
{
namespace Qrosp
{
	PluginManager::PluginManager ()
	{
		Qross::Manager::self ().registerMetaTypeHandler ("LC::Entity", EntityHandler);
		Qross::Manager::self ().addQObject (new TypesFactory, "TypesFactory");
		Qross::Manager::self ().addQObject (new UtilProxy, "Util");

		const auto& interpreters = Qross::Manager::self ().interpreters ();
		qDebug () << Q_FUNC_INFO
				<< "interpreters:"
				<< interpreters;

		const auto& plugins = FindPlugins ();
		qDebug () << Q_FUNC_INFO
				<< "found"
				<< plugins;

		for (const auto& pair : Util::Stlize (plugins))
		{
			const auto& type = pair.first;
			if (!interpreters.contains (type))
			{
				qWarning () << Q_FUNC_INFO
						<< "no interpreter for type"
						<< type
						<< interpreters;
				continue;
			}
			for (const auto& path : pair.second)
				Wrappers_ << new WrapperObject (type, path);
		}
	}

	PluginManager& PluginManager::Instance ()
	{
		static PluginManager pm;
		return pm;
	}

	void PluginManager::Release ()
	{
		Qross::Manager::self ().finalize ();
	}

	QList<QObject*> PluginManager::GetPlugins ()
	{
		return Wrappers_;
	}

	namespace
	{
		QFileInfoList Collect (const QStringList& exts, const QString& path)
		{
			QFileInfoList list;
			auto dir = QDir::home ();
			if (dir.cd (path))
				list = dir.entryInfoList (exts,
						QDir::Files |
							QDir::NoDotAndDotDot |
							QDir::Readable,
						QDir::Name);
			return list;
		}
	}

	QMap<QString, QStringList> PluginManager::FindPlugins ()
	{
		QMap<QString, QStringList> knownExtensions;
		knownExtensions ["qtscript"] << "*.es" << "*.js" << "*.qs";
		knownExtensions ["python"] << "*.py";
		knownExtensions ["ruby"] << "*.rb";

		QMap<QString, QStringList> result;

		QDir dir = QDir::home ();
		if (!dir.cd (".leechcraft/plugins/scriptable"))
			qWarning () << Q_FUNC_INFO
					<< "unable to cd into ~/.leechcraft/plugins/scriptable";
		else
		{
			const auto& dirEntries = dir.entryInfoList (QDir::Dirs |
					QDir::NoDotAndDotDot |
					QDir::Readable);
			// Iterate over the different types of scripts
			for (const auto& sameType : dirEntries)
			{
				const auto& pluginDirs = QDir { sameType.absoluteFilePath () }
						.entryInfoList (QDir::Dirs |
								QDir::NoDotAndDotDot |
								QDir::Readable);
				// For the same type iterate over subdirs with
				// actual plugins.
				for (const auto& pluginDir : pluginDirs)
				{
					const auto& type = sameType.fileName ();
					const auto& exts = knownExtensions.value (type, { "*.*" });
					const auto& list = Collect (exts, pluginDir.absoluteFilePath ());
					for (const auto& fileInfo : list)
						if (fileInfo.baseName () == pluginDir.baseName ())
							result [type] += fileInfo.absoluteFilePath ();
				}
			}
		}

		return result;
	}
}
}
