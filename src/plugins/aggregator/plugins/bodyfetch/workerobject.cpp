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

#include "workerobject.h"
#include <QUrl>
#include <QFile>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QTextCodec>
#include <QtDebug>
#include <interfaces/iscriptloader.h>
#include <plugininterface/util.h>

uint qHash (IScript_ptr script)
{
	return qHash (script.get ());
}

namespace LeechCraft
{
namespace Aggregator
{
namespace BodyFetch
{
	const int CacheValidity = 20;

	WorkerObject::WorkerObject (QObject *parent)
	: QObject (parent)
	, Inst_ (0)
	, StorageDir_ (Util::CreateIfNotExists ("aggregator/bodyfetcher/storage"))
	{
	}
	
	void WorkerObject::SetLoaderInstance (IScriptLoaderInstance *inst)
	{
		Inst_ = inst;
	}

	void WorkerObject::AppendItems (const QVariantList& items)
	{
		Items_.Val () << items;
	}
	
	void WorkerObject::ProcessItems (const QVariantList& items)
	{
		if (!Inst_)
		{
			qWarning () << Q_FUNC_INFO
					<< "null instance loader, aborting";
			return;
		}
		
		qDebug () << Q_FUNC_INFO << items.size ();

		if (LastEnumerated_.secsTo (QDateTime::currentDateTime ()) > 10)
			EnumeratedCache_.clear ();
		
		if (EnumeratedCache_.isEmpty ())
			EnumeratedCache_ = Inst_.Val ()->EnumerateScripts ();
		
		QHash<QString, IScript_ptr> channel2script;
		
		Q_FOREACH (const QVariant& item, items)
		{
			const QVariantMap& map = item.toMap ();
			
			const QString& channelLinkStr = map ["ChannelLink"].toString ();

			IScript_ptr script = channel2script.value (channelLinkStr);
			if (!script)
			{
				script = GetScriptForChannel (channelLinkStr);
				if (!script)
					continue;
				channel2script [channelLinkStr] = script;
			}
			
			QVariantList args;
			args << map ["ItemLink"];
			args << map ["ItemCommentsPageLink"];
			args << map ["ItemDescription"];
			QString fetchStr = script->InvokeMethod ("GetFullURL", args).toString ();
			if (fetchStr.isEmpty ())
				fetchStr = map ["ItemLink"].toString ();

			qDebug () << Q_FUNC_INFO << fetchStr << "using" << ChannelLink2ScriptID_ [channelLinkStr];

			const QUrl& url = QUrl::fromEncoded (fetchStr.toUtf8 ());
			URL2Script_ [url] = script;
			URL2ItemID_ [url] = map ["ItemID"].value<quint64> ();
			emit downloadRequested (url);
		}
	}
	
	IScript_ptr WorkerObject::GetScriptForChannel (const QString& channel)
	{
		IScript_ptr script;
		if (ChannelLink2ScriptID_.contains (channel))
		{
			script.reset (Inst_.Val ()->LoadScript (ChannelLink2ScriptID_ [channel]));
			if (!script->InvokeMethod ("CanHandle", QVariantList () << channel).toBool ())
			{
				ChannelLink2ScriptID_.remove (channel);
				script.reset ();
			}
		}
		
		if (!ChannelLink2ScriptID_.contains (channel))
		{
			const QString& scriptId = FindScriptForChannel (channel);
			if (scriptId.isEmpty ())
				return IScript_ptr ();

			ChannelLink2ScriptID_ [channel] = scriptId;
		}
		
		if (ChannelLink2ScriptID_ [channel].isEmpty ())
		{
			ChannelLink2ScriptID_.remove (channel);
			return IScript_ptr ();
		}

		if (!script)
			script.reset (Inst_.Val ()->LoadScript (ChannelLink2ScriptID_ [channel]));
		
		return script;
	}
	
	QString WorkerObject::FindScriptForChannel (const QString& link)
	{
		Q_FOREACH (const QString& id, EnumeratedCache_)
		{
			IScript_ptr script (Inst_.Val ()->LoadScript (id));
			if (script->InvokeMethod ("CanHandle", QVariantList () << link).toBool ())
				return id;
		}
		
		return QString ();
	}
	
	QString WorkerObject::Parse (const QString& contents, IScript_ptr script)
	{
		const QVariant& var = script->InvokeMethod ("KeepFirstTag", QVariantList ());
		
		if (var.isNull ())
			return script->InvokeMethod ("Strip", QVariantList () << contents).toString ();
		
		QStringList replacements;
		Q_FOREACH (const QVariant& varItem, var.toList ())
			replacements << varItem.toString ();
		
		return ParseWithSelectors (contents, replacements);
	}
	
	QString WorkerObject::ParseWithSelectors (const QString& contents,
			const QStringList& selectors)
	{
		QWebPage page;
		page.mainFrame ()->setHtml (contents);
		
		QString result;
		
		Q_FOREACH (const QString& sel, selectors)
		{
			QWebElement col = page.mainFrame ()->findFirstElement (sel);
			if (!col.isNull ())
				result += col.toOuterXml ().simplified ();
		}
		
		result.remove ("</br>");
		
		return result;
	}
	
	void WorkerObject::WriteFile (const QString& contents, quint64 itemId) const
	{
		QDir dir = StorageDir_;
		dir.cd (QString::number (itemId % 10));

		QFile file (dir.filePath (QString ("%1.html").arg (itemId)));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}
		
		file.write (contents.toUtf8 ());
	}
	
	QString WorkerObject::Recode (const QByteArray& rawContents) const
	{
		const QByteArray stupidCharset ("meta charset=");
		const int stupidPos = rawContents.indexOf (stupidCharset);
		
		if (stupidPos >= 0)
		{
			const int begin = stupidPos + stupidCharset.size ();
			const char sep = rawContents.at (begin);
			if (sep == '\'' || sep == '"')
			{
				const int end = rawContents.indexOf (sep, begin + 1);
				
				const QByteArray& enca = rawContents.mid (begin + 1, end - begin - 1);
				qDebug () << "detected encoding" << enca;
				QTextCodec *codec = QTextCodec::codecForName (enca);
				if (codec)
					return codec->toUnicode (rawContents);
				else
					qWarning () << Q_FUNC_INFO
							<< "unable to get codec for"
							<< enca;
			}
		}

		QTextCodec *codec = QTextCodec::codecForHtml (rawContents, 0);
		return codec ?
				codec->toUnicode (rawContents) :
				QString::fromUtf8 (rawContents);
	}

	void WorkerObject::handleDownloadFinished (QUrl url, QString filename)
	{
		IScript_ptr script = URL2Script_.take (url);
		if (!script)
		{
			qWarning () << Q_FUNC_INFO
					<< "null script for"
					<< url;
			return;
		}

		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file";
			file.remove ();
			return;
		}

		const QByteArray& rawContents = file.readAll ();
		const QString& contents = Recode (rawContents);
		file.close ();
		file.remove ();
		const QString& result = Parse (contents, script);
		
		if (result.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty result for"
					<< url;
			return;
		}
		
		const quint64 id = URL2ItemID_.take (url);
		WriteFile (result, id);
		emit newBodyFetched (id);
	}
	
	void WorkerObject::process ()
	{
		while (!Items_.Val ().isEmpty ())
		{
			QVariantList items = Items_.Val ();
			Items_.Val ().clear ();
			
			ProcessItems (items);
		}
	}
}
}
}
