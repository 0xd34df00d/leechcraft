/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sopluginloader.h"
#include <QPluginLoader>
#include <QVariantMap>
#include <QJsonObject>
#include <QtDebug>

namespace LC
{
namespace Loaders
{
	SOPluginLoader::SOPluginLoader (const QString& filename)
	: Loader_ { std::make_shared<QPluginLoader> (filename) }
	{
		Loader_->setLoadHints (QLibrary::ExportExternalSymbolsHint);
	}

	quint64 SOPluginLoader::GetAPILevel ()
	{
		return GetLibAPILevel (Loader_->fileName ());
	}

	bool SOPluginLoader::Load ()
	{
		if (IsLoaded_)
			return true;

		const auto res = Loader_->load ();
		if (!res)
			return false;

		IsLoaded_ = true;
		return true;
	}

	bool SOPluginLoader::Unload ()
	{
		if (!IsLoaded_)
		{
			qWarning () << Q_FUNC_INFO
					<< "trying to unload already loaded instance";
			return true;
		}

		IsLoaded_ = false;
		delete Loader_->instance ();
		Loader_->unload ();
		return true;
	}

	QObject* SOPluginLoader::Instance ()
	{
		return IsLoaded_ ?
				Loader_->instance () :
				nullptr;
	}

	bool SOPluginLoader::IsLoaded () const
	{
		return IsLoaded_;
	}

	QString SOPluginLoader::GetFileName () const
	{
		return Loader_->fileName ();
	}

	QString SOPluginLoader::GetErrorString () const
	{
		return Loader_->errorString ();
	}

	QVariantMap SOPluginLoader::GetManifest () const
	{
		return Loader_->metaData ().toVariantMap () ["MetaData"].toMap ();
	}
}
}
