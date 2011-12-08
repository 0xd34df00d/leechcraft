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
#include <QTimer>
#include <QApplication>
#include <QtDebug>
#include <interfaces/iscriptloader.h>
#include <util/util.h>

uint qHash (IScript_ptr script)
{
	return qHash (script.get ());
}

#if QT_VERSION < 0x040700
uint qHash (const QUrl& url)
{
	return qHash (url.toEncoded ());
}
#endif

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
	, IsProcessing_ (false)
	, RecheckScheduled_ (false)
	, StorageDir_ (Util::CreateIfNotExists ("aggregator/bodyfetcher/storage"))
	{
		QTimer *timer = new QTimer;
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (clearCaches ()));
		timer->start (10000);
	}

	void WorkerObject::SetLoaderInstance (IScriptLoaderInstance *inst)
	{
		Inst_ = inst;
	}

	bool WorkerObject::IsOk () const
	{
		return Inst_;
	}

	void WorkerObject::AppendItems (const QVariantList& items)
	{
		Items_ << items;

		QTimer::singleShot (500,
				this,
				SLOT (process ()));
	}

	void WorkerObject::ProcessItems (const QVariantList& items)
	{
		if (!Inst_)
		{
			qWarning () << Q_FUNC_INFO
					<< "null instance loader, aborting";
			return;
		}

		if (EnumeratedCache_.isEmpty ())
			EnumeratedCache_ = Inst_->EnumerateScripts ();

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
		if (CachedScripts_.contains (channel))
			return CachedScripts_ [channel];

		IScript_ptr script;
		if (ChannelLink2ScriptID_.contains (channel))
		{
			script = Inst_->LoadScript (ChannelLink2ScriptID_ [channel]);
			if (!script ||
					!script->InvokeMethod ("CanHandle", QVariantList () << channel).toBool ())
			{
				ChannelLink2ScriptID_.remove (channel);
				script.reset ();
			}
		}

		if (!ChannelLink2ScriptID_.contains (channel))
		{
			const QString& scriptId = FindScriptForChannel (channel);
			if (scriptId.isEmpty ())
			{
				CachedScripts_ [channel] = IScript_ptr ();
				return IScript_ptr ();
			}

			ChannelLink2ScriptID_ [channel] = scriptId;
		}

		if (ChannelLink2ScriptID_ [channel].isEmpty ())
		{
			ChannelLink2ScriptID_.remove (channel);
			CachedScripts_ [channel] = IScript_ptr ();
			return IScript_ptr ();
		}

		if (!script)
			script = Inst_->LoadScript (ChannelLink2ScriptID_ [channel]);

		CachedScripts_ [channel] = script;

		return script;
	}

	QString WorkerObject::FindScriptForChannel (const QString& link)
	{
		Q_FOREACH (const QString& id, EnumeratedCache_)
		{
			IScript_ptr script (Inst_->LoadScript (id));
			if (script->InvokeMethod ("CanHandle", QVariantList () << link).toBool ())
				return id;
		}

		return QString ();
	}

	namespace
	{
		QStringList GetReplacements (IScript_ptr script, const QString& method)
		{
			const QVariant& var = script->InvokeMethod (method, QVariantList ());

			QStringList result;
			Q_FOREACH (const QVariant& varItem, var.toList ())
				result << varItem.toString ();

			result.removeAll (QString ());
			result.removeDuplicates ();

			return result;
		}

		template<typename Func>
		QString ParseWithSelectors (QWebFrame *frame,
				const QStringList& selectors,
				int amount,
				Func func)
		{
			QString result;

			Q_FOREACH (const QString& sel, selectors)
			{
				QWebElementCollection col = frame->findAllElements (sel);
				for (int i = 0, size = std::min (amount, col.count ());
						i < size; ++i)
					result += func (col.at (i)).simplified ();

				qApp->processEvents ();
			}

			return result;
		}
	}

	QString WorkerObject::Parse (const QString& contents, IScript_ptr script)
	{
		const QStringList& firstTagOut = GetReplacements (script, "KeepFirstTag");
		const QStringList& allTagsOut = GetReplacements (script, "KeepAllTags");
		const QStringList& firstTagIn = GetReplacements (script, "KeepFirstTagInnerXml");

		qApp->processEvents ();

		if (firstTagOut.isEmpty () &&
				allTagsOut.isEmpty () &&
				firstTagIn.isEmpty ())
			return script->InvokeMethod ("Strip", QVariantList () << contents).toString ();

		QWebPage page;
		page.settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, false);
		page.settings ()->setAttribute (QWebSettings::JavascriptEnabled, false);
		page.settings ()->setAttribute (QWebSettings::AutoLoadImages, false);
		page.settings ()->setAttribute (QWebSettings::PluginsEnabled, false);
		page.mainFrame ()->setHtml (contents);

		qApp->processEvents ();

		QString result;
		result += ParseWithSelectors (page.mainFrame (),
				firstTagOut, 1, [] (const QWebElement& e) { return e.toOuterXml (); });
		result += ParseWithSelectors (page.mainFrame (),
				allTagsOut, 1000, [] (const QWebElement& e) { return e.toOuterXml (); });
		result += ParseWithSelectors (page.mainFrame (),
				firstTagIn, 1, [] (const QWebElement& e) { return e.toInnerXml (); });

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

	namespace
	{
		struct ProcessingGuard
		{
			bool *P_;

			ProcessingGuard (bool *p)
			: P_ (p)
			{
				*P_ = true;
			}

			~ProcessingGuard ()
			{
				*P_ = false;
			}
		};
	}

	void WorkerObject::ScheduleRechecking ()
	{
		if (RecheckScheduled_)
			return;

		QTimer::singleShot (1000,
				this,
				SLOT (recheckFinished ()));

		RecheckScheduled_ = true;
	}

	void WorkerObject::handleDownloadFinished (QUrl url, QString filename)
	{
		if (IsProcessing_)
		{
			FetchedQueue_ << qMakePair (url, filename);
			ScheduleRechecking ();
			return;
		}

		ProcessingGuard pg (&IsProcessing_);

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
		qApp->processEvents ();
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
		qApp->processEvents ();
		WriteFile (result, id);
		qApp->processEvents ();
		emit newBodyFetched (id);
	}

	void WorkerObject::recheckFinished ()
	{
		RecheckScheduled_ = false;

		if (FetchedQueue_.isEmpty ())
			return;

		if (IsProcessing_)
			ScheduleRechecking ();

		const QPair<QUrl, QString>& item = FetchedQueue_.takeFirst ();
		handleDownloadFinished (item.first, item.second);
	}

	void WorkerObject::process ()
	{
		if (Items_.isEmpty ())
			return;

		if (!IsProcessing_)
			ProcessItems (QVariantList () << Items_.takeFirst ());

		if (!Items_.isEmpty ())
			QTimer::singleShot (400,
					this,
					SLOT (process ()));
	}

	void WorkerObject::clearCaches ()
	{
		if (IsProcessing_)
			return;

		EnumeratedCache_.clear ();
		CachedScripts_.clear ();
	}
}
}
}
