/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QNetworkRequest>
#include <QRegExp>
#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QTimer>
#include <QTextCodec>
#include <QMessageBox>
#include <qwebframe.h>
#include <qwebpage.h>
#include <qwebelement.h>
#include <QCoreApplication>
#include <QMenu>
#include <QMainWindow>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "flashonclickplugin.h"
#include "flashonclickwhitelist.h"
#include "blockednetworkreply.h"
#include "userfiltersmodel.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::Poshuku::Plugins::CleanWeb;

namespace
{
	enum FilterType
	{
		FTName_,
		FTFilename_,
		FTUrl_
	};

	template<typename T>
	struct FilterFinderBase
	{
		const T& ID_;

		FilterFinderBase (const T& id)
		: ID_ (id)
		{
		}
	};

	template<FilterType>
		struct FilterFinder;

	template<>
		struct FilterFinder<FTName_> : FilterFinderBase<QString>
		{
			FilterFinder (const QString& id)
			: FilterFinderBase<QString> (id)
			{
			}

			bool operator() (const Filter& f) const
			{
				return f.SD_.Name_ == ID_;
			}
		};

	template<>
		struct FilterFinder<FTFilename_> : FilterFinderBase<QString>
		{
			FilterFinder (const QString& id)
			: FilterFinderBase<QString> (id)
			{
			}

			bool operator() (const Filter& f) const
			{
				return f.SD_.Filename_ == ID_;
			}
		};

	template<>
		struct FilterFinder<FTUrl_> : FilterFinderBase<QUrl>
		{
			FilterFinder (const QUrl& id)
			: FilterFinderBase<QUrl> (id)
			{
			}

			bool operator() (const Filter& f) const
			{
				return f.SD_.URL_ == ID_;
			}
		};

	struct LineHandler
	{
		Filter *Filter_;

		LineHandler (Filter *f)
		: Filter_ (f)
		{
		}

		void operator() (const QString& line)
		{
			if (line.size () &&
					line.at (0) == '!')
				return;

			QString actualLine;
			FilterOption f = FilterOption ();
			bool cs = false;
			if (line.indexOf ('$') != -1)
			{
				QStringList splitted = line.split ('$',
						QString::SkipEmptyParts);

				if (splitted.size () != 2)
				{
					qWarning () << Q_FUNC_INFO
						<< splitted.size ()
						<< line;
					return;
				}

				actualLine = splitted.at (0);
				QStringList options = splitted.at (1).split (',',
						QString::SkipEmptyParts);

				if (options.contains ("match-case"))
				{
					f.Case_ = Qt::CaseSensitive;
					cs = true;
				}

				Q_FOREACH (QString option, options)
					if (option.startsWith ("domain="))
					{
						QString domain = option.remove (0, 7);
						if (domain.startsWith ('~'))
							f.NotDomains_ << domain.remove (0, 1);
						else
							f.Domains_ << domain;
					}
			}
			else
				actualLine = line;

			bool white = false;
			if (actualLine.startsWith ("@@"))
			{
				actualLine.remove (0, 2);
				white = true;
			}

			if (actualLine.startsWith ('/') && 
					actualLine.endsWith ('/'))
			{
				actualLine = actualLine.mid (1, actualLine.size () - 2);
				f.MatchType_ = FilterOption::MTRegexp;
			}
			else
			{
				if (actualLine.endsWith ('|'))
				{
					actualLine.remove (0, 1);
					actualLine.prepend ('*');
				}
				else if (actualLine.startsWith ('|'))
				{
					actualLine.chop (1);
					actualLine.append ('*');
				}
				else
				{
					actualLine.prepend ('*');
					actualLine.append ('*');
				}
				actualLine.replace ('?', "\\?");
			}

			if (white)
				Filter_->ExceptionStrings_ << (cs ? actualLine : actualLine.toLower ());
			else
				Filter_->FilterStrings_ << (cs ? actualLine : actualLine.toLower ());

			if (FilterOption () != f)
				Filter_->Options_ [actualLine] = f;

			if (f.MatchType_ == FilterOption::MTRegexp)
				Filter_->RegExps_ [actualLine] = QRegExp (actualLine, f.Case_, QRegExp::RegExp);
		}
	};
};

LeechCraft::Plugins::Poshuku::Plugins::CleanWeb::Core::Core ()
: FlashOnClickPlugin_ (0)
, FlashOnClickWhitelist_ (new FlashOnClickWhitelist ())
, UserFilters_ (new UserFiltersModel (this))
{
	HeaderLabels_ << tr ("Name")
		<< tr ("Last updated")
		<< tr ("URL");
	try
	{
		LeechCraft::Util::CreateIfNotExists ("cleanweb");
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< "failed to create the directory"
			<< e.what ();
		return;
	}

	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");
	QFileInfoList infos = home.entryInfoList (QDir::Files | QDir::Readable);
	Q_FOREACH (QFileInfo info, infos)
		Parse (info.absoluteFilePath ());

	ReadSettings ();
	QTimer::singleShot (0,
			this,
			SLOT (update ()));
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	delete FlashOnClickWhitelist_;
	delete FlashOnClickPlugin_;
}

void Core::SetProxy (ICoreProxy_ptr proxy)
{
	Proxy_ = proxy;
}

ICoreProxy_ptr Core::GetProxy () const
{
	return Proxy_;
}

int Core::columnCount (const QModelIndex&) const
{
	return HeaderLabels_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () ||
			role != Qt::DisplayRole)
		return QVariant ();

	int row = index.row ();
	switch (index.column ())
	{
		case 0:
			return Filters_.at (row).SD_.Name_;
		case 1:
			return Filters_.at (row).SD_.LastDateTime_;
		case 2:
			return Filters_.at (row).SD_.URL_.toString ();
		default:
			return QVariant ();
	}
}

QVariant Core::headerData (int section, Qt::Orientation orient, int role) const
{
	if (orient != Qt::Horizontal ||
			role != Qt::DisplayRole)
		return QVariant ();

	return HeaderLabels_.at (section);
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Filters_.size ();
}

bool Core::CouldHandle (const DownloadEntity& e) const
{
	QUrl url = e.Entity_.toUrl ();
	if (url.scheme () == "abp" &&
			url.path () == "subscribe")
	{
		QString name = url.queryItemValue ("title");
		if (std::find_if (Filters_.begin (), Filters_.end (),
					FilterFinder<FTName_> (name)) == Filters_.end ())
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Core::Exists (const QString& subscrName) const
{
	return std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FTName_> (subscrName)) != Filters_.end ();
}


bool Core::Exists (const QUrl& url) const
{
	return std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FTUrl_> (url)) != Filters_.end ();
}

void Core::Handle (DownloadEntity subscr)
{
	QUrl subscrUrl = subscr.Entity_.toUrl ();

	Add (subscrUrl);
}

QAbstractItemModel* Core::GetModel ()
{
	return this;
}

void Core::Remove (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	Remove (Filters_ [index.row ()].SD_.Filename_);
}

QNetworkReply* Core::Hook (IHookProxy_ptr hook,
		QNetworkAccessManager*,
		QNetworkAccessManager::Operation*,
		QNetworkRequest *req,
		QIODevice**)
{
	if (!req->originatingObject ())
		return 0;

	if (req->url ().scheme () == "data")
	{
		qDebug () << Q_FUNC_INFO
			<< "not checking data: urls";
		return 0;
	}

	QString matched;
	if (ShouldReject (*req, &matched))
	{
		if (Blocked_.size () > 30)
			Blocked_.removeFirst ();
		qDebug () << "rejecting against" << matched << req->url ();
		Blocked_ << matched;
		hook->CancelDefault ();
		return new BlockedNetworkReply (*req, this);
	}
	return 0;
}

void Core::HandleLoadFinished (QWebPage *page)
{
	QWebFrame *frame = page->mainFrame ();
	Q_FOREACH (QString blocked, Blocked_)
	{
		QString stripped = blocked;
		stripped.remove ('\\');
		stripped.remove ('*');
		QString selector = QString ("*[src~=\"%1\"]").arg (stripped);
		QWebElementCollection elems = frame->findAllElements (selector);

		if (elems.count ())
			Blocked_.removeOne (blocked);
		else
			continue;

		qDebug () << blocked << elems.count ();

		Q_FOREACH (QWebElement elem, elems)
			elem.removeFromDocument ();
	}
}

void Core::HandleContextMenu (const QWebHitTestResult& r,
	QMenu *menu, LeechCraft::Plugins::Poshuku::PluginBase::WebViewCtxMenuStage stage)
{
	QUrl iurl = r.imageUrl ();
	if (stage == PluginBase::WVSAfterImage &&
			!iurl.isEmpty ())
		menu->addAction (tr ("Block image..."),
				UserFilters_,
				SLOT (blockImage ()))->setData (iurl);
}

UserFiltersModel* Core::GetUserFiltersModel () const
{
	return UserFilters_;
}

FlashOnClickPlugin* Core::GetFlashOnClick ()
{
	if (!FlashOnClickPlugin_)
		FlashOnClickPlugin_ = new FlashOnClickPlugin (this);
	return FlashOnClickPlugin_;
}

FlashOnClickWhitelist* Core::GetFlashOnClickWhitelist ()
{
	return FlashOnClickWhitelist_;
}

/** We test each filter until we know that we should reject it or until
 * it gets whitelisted.
 *
 * So, for each filter we first iterate through the whitelist. For each
 * entry in the whitelist:
 * - First, we check if the url's domain ends with a string from a "not
 *   apply" list if it's not empty. If it does, we skip this whitelist
 *   entry and go to the next one, if it doesn't, we continue
 *   processing.
 * - Then, if we continue processing, we check if the url's domain ends
 *   with a string from "apply list", if this list isn't empty. If it
 *   ends, we continue processing, otherwise we skip this whilelist
 *   entry and go to the next one.
 * - Then, we check if the URL matches this exception, either by regexp
 *   or wildcard. If it should be matched only in the beginning or in
 *   the end, then '*' is appended or prepended and exact match is
 *   checked. Otherwise only something is required to match. Please not
 *   that the '*' is prepended by the filter parsing code, not this one.
 *
 * The same is applied to the filter strings.
 */
bool Core::ShouldReject (const QNetworkRequest& req, QString *matchedFilter) const
{
	QUrl url = req.url ();
	QString urlStr = url.toString ();
	QString cinUrlStr = urlStr.toLower ();
	QString domain = url.host ();
	
	QList<Filter> allFilters;
	allFilters << UserFilters_->GetFilter ();
	allFilters += Filters_;
	Q_FOREACH (Filter filter, allFilters)
	{
		Q_FOREACH (QString exception, filter.ExceptionStrings_)
		{
			bool cs = filter.Options_ [exception].Case_ == Qt::CaseSensitive;
			QString url = cs ? urlStr : cinUrlStr;
			if (Matches (exception, filter, url, domain))
				return false;
		}

		Q_FOREACH (QString filterString, filter.FilterStrings_)
		{
			bool cs = filter.Options_ [filterString].Case_ == Qt::CaseSensitive;
			QString url = cs ? urlStr : cinUrlStr;
			if (Matches (filterString, filter, url, domain))
			{
				*matchedFilter = filterString;
				return true;
			}
		}
	}

	return false;
}

#if defined (Q_WS_WIN) || defined (Q_WS_MAC)
// Thanks for this goes to http://www.codeproject.com/KB/string/patmatch.aspx
bool WildcardMatches (const char *pattern, const char *str)
{
    enum State {
        Exact,        // exact match
        Any,        // ?
        AnyRepeat    // *
    };

    const char *s = str;
    const char *p = pattern;
    const char *q = 0;
    int state = 0;

    bool match = true;
    while (match && *p) {
        if (*p == '*') {
            state = AnyRepeat;
            q = p+1;
        } else if (*p == '?') state = Any;
        else state = Exact;

        if (*s == 0) break;

        switch (state) {
            case Exact:
                match = *s == *p;
                s++;
                p++;
                break;

            case Any:
                match = true;
                s++;
                p++;
                break;

            case AnyRepeat:
                match = true;
                s++;

                if (*s == *q) p++;
                break;
        }
    }

    if (state == AnyRepeat) return (*s == *q);
    else if (state == Any) return (*s == *p);
    else return match && (*s == *p);
}
#else
#include <fnmatch.h>

bool WildcardMatches (const char *pat, const char *str)
{
	return !fnmatch (pat, str, 0);
}
#endif

bool Core::Matches (const QString& exception, const Filter& filter,
		const QString& urlStr, const QString& domain) const
{
	FilterOption opt = filter.Options_ [exception];
	if (!opt.NotDomains_.isEmpty ())
	{
		bool shouldFurther = true;
		Q_FOREACH (QString notDomain, opt.NotDomains_)
			if (domain.endsWith (notDomain, opt.Case_))
			{
				shouldFurther = false;
				break;
			}
		if (!shouldFurther)
			return false;
	}

	if (!opt.Domains_.isEmpty ())
	{
		bool shouldFurther = false;
		Q_FOREACH (QString doDomain, opt.Domains_)
			if (domain.endsWith (doDomain, opt.Case_))
			{
				shouldFurther = true;
				break;
			}
		if (!shouldFurther)
			return false;
	}

	if (opt.MatchType_ == FilterOption::MTRegexp &&
			filter.RegExps_ [exception].exactMatch (urlStr))
		return true;
	else if (opt.MatchType_ == FilterOption::MTWildcard)
	{
		if (WildcardMatches (qPrintable (exception),
					qPrintable (urlStr)))
			return true;
	}
	return false;
}

void Core::HandleProvider (QObject *provider)
{
	if (Downloaders_.contains (provider))
		return;
	
	Downloaders_ << provider;
	connect (provider,
			SIGNAL (jobFinished (int)),
			this,
			SLOT (handleJobFinished (int)));
	connect (provider,
			SIGNAL (jobError (int, IDownload::Error)),
			this,
			SLOT (handleJobError (int, IDownload::Error)));
}

void Core::Parse (const QString& filePath)
{
	QFile file (filePath);
	if (!file.open (QIODevice::ReadOnly))
	{
		qWarning () << Q_FUNC_INFO
			<< "could not open file"
			<< filePath
			<< file.errorString ();
		return;
	}

	QString data = QTextCodec::codecForName ("UTF-8")->
		toUnicode (file.readAll ());
	QStringList rawLines = data.split ('\n', QString::SkipEmptyParts);
	if (rawLines.size ())
		rawLines.removeAt (0);
	QStringList lines;
	std::transform (rawLines.begin (), rawLines.end (),
			std::back_inserter (lines),
			boost::bind (&QString::trimmed,
				_1));

	Filter f;
	std::for_each (lines.begin (), lines.end (),
			LineHandler (&f));

	f.SD_.Filename_ = QFileInfo (filePath).fileName ();

	QList<Filter>::iterator pos = std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FTFilename_> (f.SD_.Filename_));
	if (pos != Filters_.end ())
	{
		int row = std::distance (Filters_.begin (), pos);
		beginRemoveRows (QModelIndex (), row, row);
		Filters_.erase (pos);
		endRemoveRows ();
		WriteSettings ();
	}

	beginInsertRows (QModelIndex (), Filters_.size (), Filters_.size ());
	Filters_ << f;
	endInsertRows ();
}

bool Core::Add (const QUrl& subscrUrl)
{
	QUrl url;
	if (subscrUrl.queryItemValue ("location").contains ("%"))
		url.setUrl (QUrl::fromPercentEncoding (subscrUrl.queryItemValue ("location").toAscii ()));
	else
		url.setUrl (subscrUrl.queryItemValue ("location"));
	QString subscrName = subscrUrl.queryItemValue ("title");

	if (Exists (subscrName) || Exists (url))
		return false;

	bool result = Load (url, subscrName);
	if (result)
	{
		QString str = tr ("The subscription %1 was successfully added.")
				.arg (subscrName);
		emit gotEntity (Util::MakeNotification ("Poshuku CleanWeb",
				str, PInfo_));
	}
	return result;
}

bool Core::Load (const QUrl& url, const QString& subscrName)
{
	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");

	QString name = QFileInfo (url.path ()).fileName ();
	QString path = home.absoluteFilePath (name);

	LeechCraft::DownloadEntity e =
		LeechCraft::Util::MakeEntity (url,
			path,
			LeechCraft::Internal |
				LeechCraft::DoNotNotifyUser |
				LeechCraft::DoNotSaveInHistory |
				LeechCraft::NotPersistent |
				LeechCraft::DoNotAnnounceEntity);

	int id = -1;
	QObject *pr;
	emit delegateEntity (e, &id, &pr);
	if (id == -1)
	{
		qWarning () << Q_FUNC_INFO
			<< "unable to delegate"
			<< subscrName
			<< url.toString ().toUtf8 ();
		QString str = tr ("The subscription %1 wasn't delegated.")
				.arg (subscrName);
		emit gotEntity (Util::MakeNotification ("Poshuku CleanWeb",
				str, PCritical_));
		return false;
	}

	HandleProvider (pr);
	PendingJob pj =
	{
		path,
		name,
		subscrName,
		url
	};
	PendingJobs_ [id] = pj;
	return true;
}

void Core::Remove (const QString& fileName)
{
	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");
	home.remove (fileName);

	QList<Filter>::iterator pos = std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FTFilename_> (fileName));
	if (pos != Filters_.end ())
	{
		int row = std::distance (Filters_.begin (), pos);
		beginRemoveRows (QModelIndex (), row, row);
		Filters_.erase (pos);
		endRemoveRows ();
		WriteSettings ();
	}
	else
		qWarning () << Q_FUNC_INFO
			<< "could not find filter for name"
			<< fileName;
}

void Core::WriteSettings ()
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_CleanWeb");
	settings.beginWriteArray ("Subscriptions");
	settings.remove ("");

	int i = 0;
	Q_FOREACH (Filter f, Filters_)
	{
		settings.setArrayIndex (i++);
		settings.setValue ("URL", f.SD_.URL_);
		settings.setValue ("name", f.SD_.Name_);
		settings.setValue ("fileName", f.SD_.Filename_);
		settings.setValue ("lastDateTime", f.SD_.LastDateTime_);
	}

	settings.endArray ();
}

void Core::ReadSettings ()
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_CleanWeb");
	int size = settings.beginReadArray ("Subscriptions");

	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		SubscriptionData sd =
		{
			settings.value ("URL").toUrl (),
			settings.value ("name").toString (),
			settings.value ("fileName").toString (),
			settings.value ("lastDateTime").toDateTime ()
		};
		if (!AssignSD (sd))
			qWarning () << Q_FUNC_INFO
				<< "could not find filter for name"
				<< sd.Filename_;
	}

	settings.endArray ();
}

bool Core::AssignSD (const SubscriptionData& sd)
{
	QList<Filter>::iterator pos =
		std::find_if (Filters_.begin (), Filters_.end (),
			FilterFinder<FTFilename_> (sd.Filename_));
	if (pos != Filters_.end ())
	{
		pos->SD_ = sd;
		int row = std::distance (Filters_.begin (), pos);
		emit dataChanged (index (row, 0), index (row, columnCount () - 1));
		return true;
	}
	else
		return false;
}

void Core::update ()
{
	if (!XmlSettingsManager::Instance ()->
			property ("Autoupdate").toBool ())
		return;

	QDateTime current = QDateTime::currentDateTime ();
	int days = XmlSettingsManager::Instance ()->
		property ("UpdateInterval").toInt ();
	Q_FOREACH (Filter f, Filters_)
		if (f.SD_.LastDateTime_.daysTo (current) > days)
			Load (f.SD_.URL_, f.SD_.Name_);
}

void Core::handleJobFinished (int id)
{
	if (!PendingJobs_.contains (id))
		return;

	PendingJob pj = PendingJobs_ [id];
	SubscriptionData sd =
	{
		pj.URL_,
		pj.Subscr_,
		pj.FileName_,
		QDateTime::currentDateTime ()
	};
	Parse (pj.FullName_);
	PendingJobs_.remove (id);
	if (!AssignSD (sd))
		qWarning () << Q_FUNC_INFO
			<< "could not find filter for name"
			<< sd.Filename_;
	WriteSettings ();
}

void Core::handleJobError (int id, IDownload::Error)
{
	if (!PendingJobs_.contains (id))
		return;

	PendingJobs_.remove (id);
}

