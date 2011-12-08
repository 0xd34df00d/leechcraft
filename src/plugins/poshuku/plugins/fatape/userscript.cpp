/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "userscript.h"
#include <algorithm>
#include <QCoreApplication>
#include <QtDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextCodec>
#include <QTextStream>
#include <util/util.h>
#include "greasemonkey.h"
#include "resourcedownloadhandler.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	const QString MetadataStart = "// ==UserScript==";
	const QString MetadataEnd = "// ==/UserScript==";

	template<typename InputIterator, typename Function>
	bool any (InputIterator first, InputIterator last, Function f)
	{
		return std::find_if (first, last, f) != last;
	}

	UserScript::UserScript (const QString& scriptPath)
	: ScriptPath_ (scriptPath)
	, MetadataRX_ ("//\\s+@(\\S*)\\s+(.*)", Qt::CaseInsensitive)
	{
		ParseMetadata ();
		if (!Metadata_.count ("include"))
			Metadata_.insert ("include", "*");

		QSettings settings(QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");

		Enabled_ = !settings.value (QString("disabled/%1%2")
				.arg (qHash (Namespace ()))
				.arg (qHash (Name ())), false).toBool ();;
	}

	UserScript::UserScript (const UserScript& script)
	: MetadataRX_ ("//\\s+@(\\S*)\\s+(.*)", Qt::CaseInsensitive)
	{
		ScriptPath_ = script.ScriptPath_;
		Metadata_ = script.Metadata_;
		Enabled_ = script.Enabled_;
	}

	void UserScript::ParseMetadata ()
	{
		QFile script (ScriptPath_);

		if (!script.open (QFile::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to open file"
				<< script.fileName ()
				<< "for reading:"
				<< script.errorString ();
			return;
		}

		QTextStream content (&script);
		QString line;

		content.setCodec (QTextCodec::codecForName ("UTF-8"));
		if (content.readLine () != MetadataStart)
			return;

		while ((line = content.readLine ()) != MetadataEnd && !content.atEnd ())
		{
			MetadataRX_.indexIn (line);
			QString key (MetadataRX_.cap (1).trimmed ());
			QString value (MetadataRX_.cap (2).trimmed ());

			Metadata_.insert (key, value);
		}
	}

	void UserScript::BuildPatternsList (QList<QRegExp>& list, bool include) const
	{
		Q_FOREACH (const QString& pattern,
				Metadata_.values (include ? "include" : "exclude"))
			list.append (QRegExp (pattern, Qt::CaseInsensitive, QRegExp::Wildcard));
	}

	bool UserScript::MatchToPage (const QString& pageUrl) const
	{
		QList<QRegExp> include;
		QList<QRegExp> exclude;
		auto match = [pageUrl] (QRegExp& rx)
				{ return rx.indexIn (pageUrl, 0, QRegExp::CaretAtZero) != -1; };

		BuildPatternsList (include);
		BuildPatternsList (exclude, false);

		return any (include.begin (), include.end (), match) &&
				!any (exclude.begin (), exclude.end (), match);
	}

	void UserScript::Inject (QWebFrame *frame, IProxyObject *proxy) const
	{
		if (!Enabled_)
			return;

		QFile script (ScriptPath_);

		if (!script.open (QFile::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to open file"
				<< script.fileName ()
				<< "for reading:"
				<< script.errorString ();
			return;
		}

		QTextStream content (&script);
		QString gmLayerId = QString ("Greasemonkey%1%2")
				.arg (qHash (Namespace ()))
				.arg (qHash (Name ()));
		QString toInject = QString ("(function (){"
			"var GM_addStyle = %1.addStyle;"
			"var GM_deleteValue = %1.deleteValue;"
			"var GM_getValue = %1.getValue;"
			"var GM_listValues = %1.listValues;"
			"var GM_setValue = %1.setValue;"
			"var GM_openInTab = %1.openInTab;"
			"var GM_getResourceText = %1.getResourceText;"
			"var GM_getResourceURL = %1.getResourceURL;"
			"var GM_log = function(){console.log.apply(console, arguments)};"
			"%2})()")
				.arg (gmLayerId)
				.arg (content.readAll ());

		frame->addToJavaScriptWindowObject (gmLayerId,
				new GreaseMonkey (frame, proxy, *this));
		frame->evaluateJavaScript (toInject);
	}

	QString UserScript::Name () const
	{
		return Metadata_.value ("name", QFileInfo (ScriptPath_).baseName ());
	}

	QString UserScript::Description () const
	{
		return Metadata_.value ("description");
	}

	QString UserScript::Namespace () const
	{
		return Metadata_.value ("namespace", "Default namespace");
	}

	QString UserScript::GetResourcePath (const QString& resourceName) const
	{
		const QString& resource = QStringList (Metadata_.values ("resource"))
				.filter (QRegExp (QString ("%1\\s.*").arg (resourceName)))
				.value (0)
				.mid (resourceName.length ())
				.trimmed ();
		QUrl resourceUrl (resource);
		const QString& resourceFile = QFileInfo (resourceUrl.path ()).fileName ();

		return resourceFile.isEmpty () ?
			QString () :
			QFileInfo (Util::CreateIfNotExists ("data/poshuku/fatape/scripts/resources"),
				QString ("%1%2_%3")
					.arg (qHash (Namespace ()))
					.arg (qHash (Name ()))
					.arg (resourceFile)).absoluteFilePath ();
	}

	QString UserScript::Path() const
	{
		return ScriptPath_;
	}

	bool UserScript::IsEnabled () const
	{
		return Enabled_;
	}


	void UserScript::SetEnabled (bool value)
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");

		settings.setValue (QString ("disabled/%1%2")
			.arg (qHash (Namespace ()))
			.arg (qHash (Name ())), !value);
		Enabled_ = value;

	}

	void UserScript::Delete ()
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");

		settings.remove (QString ("storage/%1/%2")
				.arg (qHash (Namespace ()))
				.arg (Name ()));
		settings.remove (QString ("resources/%1/%2")
				.arg (qHash (Namespace ()))
				.arg (Name ()));
		settings.remove (QString ("disabled/%1%2")
				.arg (qHash (Namespace ()))
				.arg (Name ()));
		Q_FOREACH (const QString& resource, Metadata_.values ("resource"))
			QFile::remove (GetResourcePath (resource.mid (0, resource.indexOf (" "))));
		QFile::remove (ScriptPath_);
	}

	QStringList UserScript::Include () const
	{
		return Metadata_.values ("include");
	}

	QStringList UserScript::Exclude() const
	{
		return Metadata_.values ("exclude");
	}

	void UserScript::Install (QNetworkAccessManager *networkManager)
	{
		const QString& temp = QDesktopServices::storageLocation (QDesktopServices::TempLocation);

		if (!ScriptPath_.startsWith (temp))
			return;

		QFile tempScript (ScriptPath_);
		QFileInfo installPath (Util::CreateIfNotExists ("data/poshuku/fatape/scripts/"),
				QFileInfo(ScriptPath_).fileName ());

		tempScript.copy (installPath.absoluteFilePath ());
		ScriptPath_ = installPath.absoluteFilePath ();
		Q_FOREACH (const QString& resource, Metadata_.values ("resource"))
			DownloadResource (resource, networkManager);
		Q_FOREACH (const QString& required, Metadata_.values ("require"))
			DownloadRequired (required, networkManager);
	}

	void UserScript::DownloadResource (const QString& resource,
			QNetworkAccessManager *networkManager)
	{
		const QString& resourceName = resource.mid (0, resource.indexOf (" "));
		const QString& resourceUrl = resource.mid (resource.indexOf (" ") + 1);
		QNetworkRequest resourceRequest;

		resourceRequest.setUrl (QUrl (resourceUrl));
		QNetworkReply *reply = networkManager->get (resourceRequest);
		QObject::connect (reply,
				SIGNAL (finished ()),
				new ResourceDownloadHandler (resourceName, this, reply),
				SLOT (handleFinished ()));
	}

	void UserScript::DownloadRequired (const QString& required,
			QNetworkAccessManager *networkManager)
	{
		//TODO
	}
}
}
}

