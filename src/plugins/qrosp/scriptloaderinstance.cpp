/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

	QObject* ScriptLoaderInstance::GetObject ()
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
