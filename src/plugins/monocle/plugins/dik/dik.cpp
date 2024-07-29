/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dik.h"
#include <QIcon>
#include <util/util.h>
#include "document.h"

namespace LC
{
namespace Monocle
{
namespace Dik
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("monocle_dik");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.Dik";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle Dik";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("MOBI backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { PluginClass };
	}

	bool Plugin::CanLoadDocument (const QString& file)
	{
		const auto& lower = file.toLower ();
		return lower.endsWith (".mobi") ||
				lower.endsWith (".prc");
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		return IDocument_ptr (new Document (file, this));
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "application/x-mobipocket-ebook", "application/x-mobipocket" };
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("MOBI books"),
				{ "mobi" }
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_dik, LC::Monocle::Dik::Plugin);
