/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scriptloaderinstance.h"
#include <QDir>
#include <QtDebug>
#include <qross/core/manager.h>
#include "loadedscript.h"

namespace LC
{
namespace Qrosp
{
	const QString CommentSeparator = "#@#@";

	ScriptLoaderInstance::ScriptLoaderInstance (const QString& relPath, QObject *parent)
	: QObject (parent)
	, Loader_ ("scripts/" + relPath)
	{
	}

	QObject* ScriptLoaderInstance::GetQObject ()
	{
		return this;
	}

	void ScriptLoaderInstance::AddGlobalPrefix ()
	{
		Loader_.AddGlobalPrefix ();
	}

	void ScriptLoaderInstance::AddLocalPrefix (QString prefix)
	{
		Loader_.AddLocalPrefix (prefix);
	}

	QStringList ScriptLoaderInstance::EnumerateScripts () const
	{
		ID2Interpereter_.clear ();

		const auto& interpreters = Qross::Manager::self ().interpreters ();

		QStringList result;
		for (const auto& interp : interpreters)
			for (const auto& entry : Loader_.List (interp, {}, QDir::AllEntries | QDir::NoDotAndDotDot))
			{
				const auto& id = entry.filePath ();
				ID2Interpereter_ [id] = interp;
				result << id;
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
			return {};
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
		if (ID2Interpereter_.contains (id))
			return std::make_shared<LoadedScript> (id, ID2Interpereter_ [id]);

		const QFileInfo info { id };
		if (!info.isAbsolute ())
		{
			qWarning () << Q_FUNC_INFO
					<< "non-absolute file path passed:"
					<< id;
			return {};
		}

		auto interp = Qross::Manager::self ().interpreternameForFile (id);

		if (interp.isEmpty ())
		{
			static const QMap<QString, QString> knownInterpreters
			{
				{ "js", "qtscript" },
				{ "qs", "qtscript" },
				{ "es", "qtscript" },
				{ "py", "python" },
				{ "rb", "ruby" }
			};

			interp = knownInterpreters [info.suffix ()];
			if (!Qross::Manager::self ().interpreters ().contains (interp))
				interp.clear ();
		}

		if (interp.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot guess interpreter for file"
					<< id;
			return {};
		}

		return std::make_shared<LoadedScript> (id, interp);
	}
}
}
