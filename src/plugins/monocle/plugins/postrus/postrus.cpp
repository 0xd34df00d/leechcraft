/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "postrus.h"
#include <QIcon>
#include <util/sys/mimedetector.h>
#include <util/util.h>
#include "redirector.h"

namespace LC
{
namespace Monocle
{
namespace Postrus
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("monocle_postrus");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.Postrus";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle Postrus";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("PostScript backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	auto Plugin::CanLoadDocument (const QString& file) -> LoadCheckResult
	{
		const auto& mime = Util::MimeDetector {} (file);
		return mime == "application/postscript" ?
				LoadCheckResult::Redirect :
				LoadCheckResult::Cannot;
	}

	IDocument_ptr Plugin::LoadDocument (const QString&)
	{
		return {};
	}

	IRedirectProxy_ptr Plugin::GetRedirection (const QString& filename)
	{
		return std::make_shared<Redirector> (filename);
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "application/postscript" };
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("PostScript files"),
				{ "ps", "eps" }
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_postrus, LC::Monocle::Postrus::Plugin);
