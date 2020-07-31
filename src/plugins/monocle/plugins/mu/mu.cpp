/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mu.h"
#include <QIcon>
#include "document.h"

namespace LC
{
namespace Monocle
{
namespace Mu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		MuCtx_ = fz_new_context (0, 0, FZ_STORE_DEFAULT);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.Mu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return QString::fromUtf8 ("Monocle μ");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("PDF backend for Monocle using the mupdf library.");
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
		return file.toLower ().endsWith (".pdf") ?
				LoadCheckResult::Can :
				LoadCheckResult::Cannot;
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		return IDocument_ptr (new Document (file, MuCtx_, this));
	}

	QStringList Plugin::GetSupportedMimes () const
	{
		return { "application/pdf" };
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_mu, LC::Monocle::Mu::Plugin);
