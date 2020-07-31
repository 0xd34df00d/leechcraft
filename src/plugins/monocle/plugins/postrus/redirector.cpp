/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "redirector.h"
#include <QTimer>
#include <QProcess>
#include <QTemporaryFile>
#include <QtDebug>
#include <QDir>

namespace LC
{
namespace Monocle
{
namespace Postrus
{
	Redirector::Redirector (const QString& src)
	: Source_ { src }
	, Process_ { new QProcess { this } }
	{
		QTimer::singleShot (0,
				this,
				&Redirector::StartConverting);
	}

	QObject* Redirector::GetQObject ()
	{
		return this;
	}

	QString Redirector::GetRedirectSource () const
	{
		return Source_;
	}

	QString Redirector::GetRedirectTarget () const
	{
		return {};
	}

	QString Redirector::GetRedirectedMime () const
	{
		return "application/pdf";
	}

	void Redirector::StartConverting ()
	{
		{
			QTemporaryFile file { QDir::tempPath () + "/lc_monocle_postrus.XXXXXX.pdf" };
			if (!file.open ())
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to create a temporarty file"
						<< file.fileName ()
						<< file.errorString ();
				emit ready (Target_);
				return;
			}

			Target_ = file.fileName ();
		}

		qDebug () << Q_FUNC_INFO
				<< Source_
				<< Target_;
		Process_->start ("ps2pdf", { "-dPDFSETTINGS=/prepress", "-dEmbedAllFonts=true", "-dSubsetFonts=false", "-r600", Source_, Target_ });
		connect (Process_,
				qOverload<int, QProcess::ExitStatus> (&QProcess::finished),
				this,
				&Redirector::HandleFinished);
	}

	void Redirector::HandleFinished ()
	{
		qDebug () << Q_FUNC_INFO
				<< Process_->exitStatus ()
				<< Process_->exitCode ();
		if (Process_->exitCode ())
			qWarning () << Q_FUNC_INFO
					<< Process_->readAllStandardError ();

		emit ready (Target_);
	}
}
}
}
