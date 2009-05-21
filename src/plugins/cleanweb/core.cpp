#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QNetworkRequest>
#include <QRegExp>
#include <QFile>
#include <QSettings>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
#include <plugininterface/util.h>
#include <plugininterface/proxy.h>

using namespace LeechCraft::Plugins::CleanWeb;
using LeechCraft::Util::Proxy;

namespace
{
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
					f.Case_ = Qt::CaseSensitive;
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
				f.MatchType_ = FilterOption::MTRegexp_;
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
				Filter_->ExceptionStrings_ << actualLine;
			else
				Filter_->FilterStrings_ << actualLine;

			if (FilterOption () != f)
				Filter_->Options_ [actualLine] = f;

			QRegExp::PatternSyntax syntax = (f.MatchType_ == FilterOption::MTRegexp_ ?
					QRegExp::RegExp : QRegExp::Wildcard);
			Filter_->RegExps_ [actualLine] = QRegExp (actualLine, f.Case_, syntax);
		}
	};
};

FilterOption::FilterOption ()
: Case_ (Qt::CaseInsensitive)
, MatchType_ (MTWildcard_)
{
}

bool LeechCraft::Plugins::CleanWeb::operator== (const FilterOption& f1, const FilterOption& f2)
{
	return f1.Case_ == f2.Case_ &&
		f1.MatchType_ == f2.MatchType_ &&
		f1.Domains_ == f2.Domains_ &&
		f1.NotDomains_ == f2.NotDomains_;
}

bool LeechCraft::Plugins::CleanWeb::operator!= (const FilterOption& f1, const FilterOption& f2)
{
	return !(f1 == f2);
}

Core::Core ()
{
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
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
}

void Core::Handle (DownloadEntity subscr)
{
	QString urlString = QTextCodec::codecForName ("UTF-8")->
		toUnicode (subscr.Entity_);
	QUrl subscrUrl (urlString);
	QUrl url (subscrUrl.queryItemValue ("location"));
	QString subscrName = subscrUrl.queryItemValue ("title");

	QDir home = QDir::home ();
	home.cd (".leechcraft");
	home.cd ("cleanweb");

	QString name = QFileInfo (url.path ()).fileName ();
	QString path = home.absoluteFilePath (name);

	LeechCraft::DownloadEntity e =
	{
		url.toString ().toUtf8 (),
		path,
		QString (),
		LeechCraft::Internal |
			LeechCraft::DoNotNotifyUser |
			LeechCraft::DoNotSaveInHistory |
			LeechCraft::NotPersistent |
			LeechCraft::DoNotAnnounceEntity,
		QVariant ()
	};

	int id = -1;
	QObject *pr;
	emit delegateEntity (e, &id, &pr);
	if (id == -1)
	{
		QMessageBox::critical (0,
				tr ("Error"),
				tr ("Job the subscription wasn't delegated."));
		qWarning () << Q_FUNC_INFO
			<< url.toString ().toUtf8 ();
		return;
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
}

QNetworkReply* Core::Hook (IHookProxy*,
		QNetworkAccessManager::Operation*,
		QNetworkRequest *req,
		QIODevice**)
{
	if (ShouldReject (*req))
	{
		qDebug () << "rejecting" << req->url ();
		*req = QNetworkRequest ();
	}
	return 0;
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
bool Core::ShouldReject (const QNetworkRequest& req) const
{
	QUrl url = req.url ();
	QString urlStr = url.toString ();
	QString domain = url.host ();
	
	Q_FOREACH (Filter filter, Filters_)
	{
		Q_FOREACH (QString exception, filter.ExceptionStrings_)
			if (Matches (exception, filter, urlStr, domain))
				return false;

		Q_FOREACH (QString filterString, filter.FilterStrings_)
			if (Matches (filterString, filter, urlStr, domain))
				return true;
	}

	return false;
}

#ifdef Q_WS_WIN
#include <windows.h>

BOOL szWildMatch6(PSZ pat, PSZ str)
{
	int i;
	BOOL star = FALSE;

loopStart:
	for (i = 0; str[i]; i++)
	{
		switch (pat[i])
		{
			case '?':
				if (str[i] == '.')
					goto starCheck;
				break;
			case '*':
				star = TRUE;
				str += i, pat += i;
				do
				{
					++pat;
				}
				while (*pat == '*');
				if (!*pat)
					return TRUE;
				goto loopStart;
			default:
				if (mapCaseTable[str[i]] != mapCaseTable[pat[i]])
				   goto starCheck;
				break;
		} /* endswitch */
	} /* endfor */
	while (pat[i] == '*') ++i;
	return (!pat[i]);

starCheck:
	if (!star)
		return FALSE;
	str++;
	goto loopStart;
}

bool WildcardMatches (const char *pat, const char *str)
{
	return szWildMatch6 (pat, str);
}
#elif Q_WS_MAC
#error "Sorry, we don't provide wildcard matching for Macs now."
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

	if (opt.MatchType_ == FilterOption::MTRegexp_ &&
			filter.RegExps_ [exception].exactMatch (urlStr))
		return true;
	else if (opt.MatchType_ == FilterOption::MTWildcard_)
	{
		if (opt.Case_ == Qt::CaseSensitive)
		{
			if (WildcardMatches (qPrintable (exception), qPrintable (urlStr)))
				return true;
		}
		else
		{
			if (WildcardMatches (qPrintable (exception.toLower ()),
						qPrintable (urlStr.toLower ())))
				return true;
		}
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
	Filters_ << f;
}

void Core::WriteSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CleanWeb");
	settings.beginGroup ("Subscriptions");

	QMap<QString, QVariant> tmp;

	Q_FOREACH (QString key, File2Url_.keys ())
		tmp [key] = File2Url_ [key];
	settings.setValue ("MapUrl", tmp);

	Q_FOREACH (QString key, File2Name_.keys ())
		tmp [key] = File2Name_ [key];
	settings.setValue ("MapName", tmp);

	settings.endGroup ();
}

void Core::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CleanWeb");
	settings.beginGroup ("Subscriptions");

	QMap<QString, QVariant> tmp = settings.value ("MapUrl").toMap ();
	Q_FOREACH (QString key, tmp.keys ())
		File2Url_ [key] = tmp [key].toUrl ();

	tmp = settings.value ("MapName").toMap ();
	Q_FOREACH (QString key, tmp.keys ())
		File2Name_ [key] = tmp [key].toString ();

	settings.endGroup ();
}

void Core::handleJobFinished (int id)
{
	PendingJob pj = PendingJobs_ [id];
	File2Url_ [pj.FileName_] = pj.URL_;
	File2Name_ [pj.FileName_] = pj.Subscr_;
	Parse (pj.FullName_);
	PendingJobs_.remove (id);
	WriteSettings ();
}

void Core::handleJobError (int id, IDownload::Error)
{
	PendingJobs_.remove (id);
}

