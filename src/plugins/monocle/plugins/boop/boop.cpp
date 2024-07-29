/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "boop.h"
#include <QIcon>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include "document.h"

namespace LC::Monocle::Boop
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.Boop";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle Boop";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("ePub backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Monocle.IBackendPlugin" };
	}

	bool Plugin::CanLoadDocument (const QString& file)
	{
		return file.endsWith (".epub"_ql, Qt::CaseInsensitive);
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		return LoadZip (file, this);
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "application/epub+zip" };
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("ePub files"),
				{ "epub" }
			}
		};
	}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_boop, LC::Monocle::Boop::Plugin);
