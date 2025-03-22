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
#include <util/sll/qtutil.h>
#include <util/sys/mimedetector.h>
#include <util/threads/coro.h>

namespace LC::Monocle::Postrus
{
	void Plugin::Init (ICoreProxy_ptr)
	{
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
		return "Monocle Postrus"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("PostScript backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { PluginClass };
	}

	bool Plugin::CanRedirectDocument (const QString& file) const
	{
		const auto& mime = Util::MimeDetector {} (file);
		return mime == "application/postscript";
	}

	QString Plugin::GetRedirectionMime (const QString& filename) const
	{
		return CanRedirectDocument (filename) ? "application/pdf"_qs : QString {};
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
		process.start ("ps2pdf"_qs,
				{
					"-dPDFSETTINGS=/prepress"_qs,
					"-dEmbedAllFonts=true"_qs,
					"-dSubsetFonts=false"_qs,
					"-r600"_qs,
					filename,
					*target
				});
		co_await process;

		qDebug () << process.exitStatus () << process.exitCode () << process.error ();

		if (process.exitStatus () == QProcess::NormalExit && !process.exitCode ())
			co_return RedirectionResult { .TargetPath_ = *target };

		co_return {};
	}

	QList<IKnowFileExtensions::ExtInfo> Plugin::GetKnownFileExtensions () const
	{
		return
		{
			{
				tr ("PostScript files"),
				{ "ps"_qs, "eps"_qs }
			}
		};
	}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_postrus, LC::Monocle::Postrus::Plugin);
