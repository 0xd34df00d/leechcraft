#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QNetworkRequest>
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QTextCodec>
#include <plugininterface/util.h>

using namespace LeechCraft::Plugins::CleanWeb;

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
			else if (actualLine.endsWith ('|'))
			{
				actualLine.remove (0, 1);
				actualLine = QRegExp::escape (actualLine);
				actualLine.prepend ('*');
				f.MatchType_ = FilterOption::MTSide_;
			}
			else if (actualLine.startsWith ('|'))
			{
				actualLine.chop (1);
				actualLine = QRegExp::escape (actualLine);
				actualLine.append ('*');
				f.MatchType_ = FilterOption::MTSide_;
			}
			else
				actualLine = QRegExp::escape (actualLine);

			if (white)
				Filter_->ExceptionStrings_ << actualLine;
			else
				Filter_->FilterStrings_ << actualLine;

			if (FilterOption () != f)
				Filter_->Options_ [actualLine] = f;
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
	{
		QFile file (info.absoluteFilePath ());
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "could not open file"
				<< info.absoluteFilePath ()
				<< file.errorString ();
			continue;
		}

		QString data = QTextCodec::codecForName ("UTF-8")->
			toUnicode (file.readAll ());
		QStringList rawLines = data.split ('\n', QString::SkipEmptyParts);
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
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
}

QNetworkReply* Core::Hook (IHookProxy *proxy,
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
			if (Matches (exception, filter.Options_ [exception],
						urlStr, domain))
				return false;

		Q_FOREACH (QString filterString, filter.FilterStrings_)
			if (Matches (filterString, filter.Options_ [filterString],
						urlStr, domain))
				return true;
	}

	return false;
}

bool Core::Matches (const QString& exception, const FilterOption& opt,
		const QString& urlStr, const QString& domain) const
{
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

	QRegExp re (exception, opt.Case_);
	if (opt.MatchType_ != FilterOption::MTRegexp_)
		re.setPatternSyntax (QRegExp::Wildcard);

	bool exact = re.exactMatch (urlStr);
	if ((opt.MatchType_ == FilterOption::MTSide_ &&
				exact) ||
			re.matchedLength ())
		return true;
	return false;
}

