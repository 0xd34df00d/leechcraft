/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "postrus.h"
#include <QDir>
#include <QIcon>
#include <QProcess>
#include <QTemporaryFile>
#include <util/threads/coro/process.h>
#include <util/sys/mimedetector.h>
#include <util/util.h>

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

	QString Plugin::GetRedirectionMime (const QString& filename)
	{
		return CanLoadDocument (filename) == LoadCheckResult::Redirect ? "application/pdf" : QString {};
	}

	namespace
	{
		std::optional<QString> GetTemporaryName ()
		{
			QTemporaryFile file { QDir::tempPath () + "/lc_monocle_postrus.XXXXXX.pdf" };
			if (!file.open ())
			{
				qWarning () << "unable to create a temporarty file" << file.fileName () << file.errorString ();
				return {};
			}
			return file.fileName ();
		}
	}

	Util::Task<std::optional<RedirectionResult>> Plugin::GetRedirection (const QString& filename)
	{
		const auto& target = GetTemporaryName ();
		if (!target)
			co_return {};

		qDebug () << filename << *target;
		QProcess process;
		process.start ("ps2pdf", { "-dPDFSETTINGS=/prepress", "-dEmbedAllFonts=true", "-dSubsetFonts=false", "-r600", filename, *target });
		co_await process;

		qDebug () << process.exitStatus () << process.exitCode () << process.error ();

		if (process.exitStatus () == QProcess::NormalExit && !process.exitCode ())
			co_return RedirectionResult { .TargetPath_ = *target };

		co_return {};
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
