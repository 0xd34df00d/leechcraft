/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "scriptloaderinstance.h"
#include <QDir>
#include <QtDebug>
#include <qross/core/manager.h>
#include "loadedscript.h"

namespace LeechCraft
{
namespace Qrosp
{
	const QString CommentSeparator = "#@#@";

	ScriptLoaderInstance::ScriptLoaderInstance (const QString& relPath, QObject *parent)
	: QObject (parent)
	, RelativePath_ (relPath)
	{
		if (!RelativePath_.endsWith ('/'))
			RelativePath_ += '/';
	}

	QObject* ScriptLoaderInstance::GetQObject ()
	{
		return this;
	}

	void ScriptLoaderInstance::AddGlobalPrefix ()
	{
#ifdef Q_OS_MAC
			Prefixes_ << QString (QApplication::applicationDirPath () + "/../Resources/scripts/");
#elif defined (Q_OS_WIN32)
			Prefixes_ << QString (QApplication::applicationDirPath () + "/share/scripts/");
#elif defined (INSTALL_PREFIX)
			Prefixes_ << QString (INSTALL_PREFIX "/share/leechcraft/scripts/");
#else
			Prefixes_ << "/usr/local/share/leechcraft/scripts/"
					<< "/usr/share/leechcraft/scripts/";
#endif
	}

	void ScriptLoaderInstance::AddLocalPrefix (QString prefix)
	{
		if (!prefix.isEmpty () &&
				!prefix.endsWith ('/'))
			prefix.append ('/');
		Prefixes_ << QDir::homePath () + "/.leechcraft/data/scripts/" + prefix;
	}

	QStringList ScriptLoaderInstance::EnumerateScripts () const
	{
		ID2Interpereter_.clear ();

		const QStringList& interpreters = Qross::Manager::self ().interpreters ();
		QMap<QString, QStringList> knownExtensions;
		knownExtensions ["qtscript"] << "*.es" << "*.js" << "*.qs";
		knownExtensions ["python"] << "*.py";
		knownExtensions ["ruby"] << "*.rb";

		QStringList result;
		Q_FOREACH (const QString& prefix, Prefixes_)
			Q_FOREACH (const QString& interp, interpreters)
			{
				const QString& path = prefix + RelativePath_ + interp + '/';

				QDir dir (path);
				const QStringList& entries = dir.entryList (knownExtensions [interp],
						QDir::Readable | QDir::Files);

				Q_FOREACH (const QString& entry, entries)
				{
					const QString& id = path + entry;
					ID2Interpereter_ [id] = interp;
					result << id;
				}
			}

		return result;
	}

	QVariantMap ScriptLoaderInstance::GetScriptInfo (const QString& id)
	{
		QFile file (id);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< id;
			return QVariantMap ();
		}

		QVariantMap result;
		for (int i = 0; i < 20; ++i)
		{
			const QString& read = QString::fromUtf8 (file.readLine ());
			const int pos = read.indexOf (CommentSeparator);
			if (pos == -1)
				continue;

			const QString& data = read.mid (pos + CommentSeparator.length ()).trimmed ();
			const int sepPos = data.indexOf (':');
			if (sepPos <= 0)
			{
				qWarning () << Q_FUNC_INFO
						<< "malformed comment"
						<< data;
				continue;
			}

			result [data.left (sepPos).trimmed ()] = data.mid (sepPos + 1).trimmed ();
		}
		return result;
	}

	IScript_ptr ScriptLoaderInstance::LoadScript (const QString& id)
	{
		return IScript_ptr (new LoadedScript (id, ID2Interpereter_ [id]));
	}
}
}
