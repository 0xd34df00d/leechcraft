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

#include "core.h"
#include <algorithm>
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
#include <qgraphicswebview.h>
#include <util/util.h>
#include <util/customnetworkreply.h>
#include "xmlsettingsmanager.h"
#include "flashonclickplugin.h"
#include "flashonclickwhitelist.h"
#include "userfiltersmodel.h"
#include "lineparser.h"

Q_DECLARE_METATYPE (QWebFrame*);
Q_DECLARE_METATYPE (QPointer<QWebFrame>);

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
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
	};

	Core::Core ()
	: FlashOnClickPlugin_ (0)
	, FlashOnClickWhitelist_ (new FlashOnClickWhitelist ())
	, UserFilters_ (new UserFiltersModel (this))
	{
		qRegisterMetaType<QWebFrame*> ("QWebFrame*");
		qRegisterMetaType<QPointer<QWebFrame>> ("QPointer<QWebFrame>");

		HeaderLabels_ << tr ("Name")
			<< tr ("Last updated")
			<< tr ("URL");
		try
		{
			Util::CreateIfNotExists ("cleanweb");
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

		connect (UserFilters_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
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

	bool Core::CouldHandle (const Entity& e) const
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

	void Core::Handle (Entity subscr)
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

	namespace
	{
		void RemoveElem (QWebElement elem)
		{
			auto parent = elem.parent ();
			elem.removeFromDocument ();
			if (!parent.findAll ("*").count ())
				RemoveElem (parent);
		}
	}

	void Core::HandleInitialLayout (QWebPage*, QWebFrame *frame)
	{
		QMetaObject::invokeMethod (this,
				"handleFrameLayout",
				Qt::QueuedConnection,
				Q_ARG (QPointer<QWebFrame>, QPointer<QWebFrame> (frame)));
	}

	QNetworkReply* Core::Hook (IHookProxy_ptr hook,
			QNetworkAccessManager*,
			QNetworkAccessManager::Operation*,
			QIODevice**)
	{
		QNetworkRequest req = hook->GetValue ("request").value<QNetworkRequest> ();
		if (!req.originatingObject ())
			return 0;

		if (req.url ().scheme () == "data")
			return 0;

		QString matched;
		if (!ShouldReject (req, &matched))
			return 0;

		hook->CancelDefault ();

		QWebFrame *frame = qobject_cast<QWebFrame*> (req.originatingObject ());
		qDebug () << "rejecting against" << matched << frame;
		if (frame)
			QMetaObject::invokeMethod (this,
					"delayedRemoveElements",
					Qt::QueuedConnection,
					Q_ARG (QPointer<QWebFrame>, frame),
					Q_ARG (QString, req.url ().toEncoded ()));

		Util::CustomNetworkReply *result = new Util::CustomNetworkReply (req.url (), this);
		result->SetContent (QString ("Blocked by Poshuku CleanWeb"));
		result->SetError (QNetworkReply::ContentAccessDenied,
				tr ("Blocked by Poshuku CleanWeb: %1")
					.arg (req.url ().toString ()));
		hook->SetReturnValue (QVariant::fromValue<QNetworkReply*> (result));
		return result;
	}

	void Core::HandleExtension (LeechCraft::IHookProxy_ptr proxy,
			QWebPage *page,
			QWebPage::Extension ext,
			const QWebPage::ExtensionOption *opt,
			QWebPage::ExtensionReturn*)
	{
		if (ext != QWebPage::ErrorPageExtension)
			return;

		auto error = static_cast<const QWebPage::ErrorPageExtensionOption*> (opt);
		if (error->error != QNetworkReply::ContentAccessDenied)
			return;

		QString url = error->url.toEncoded ();
		proxy->CancelDefault ();
		proxy->SetReturnValue (true);
		QMetaObject::invokeMethod (this,
				"delayedRemoveElements",
				Qt::QueuedConnection,
				Q_ARG (QPointer<QWebFrame>, page->mainFrame ()),
				Q_ARG (QString, url));
	}

	void Core::HandleContextMenu (const QWebHitTestResult& r,
		QGraphicsWebView *view, QMenu *menu,
		LeechCraft::Poshuku::WebViewCtxMenuStage stage)
	{
		QUrl iurl = r.imageUrl ();
		if (stage == WVSAfterImage &&
				!iurl.isEmpty ())
		{
			QAction *action = menu->addAction (tr ("Block image..."),
					UserFilters_,
					SLOT (blockImage ()));
			action->setProperty ("CleanWeb/URL", iurl);
			action->setProperty ("CleanWeb/View", QVariant::fromValue<QObject*> (view));
		}
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
		if (!req.hasRawHeader ("referer"))
			return false;

		const QUrl& url = req.url ();
		const QString& urlStr = url.toString ();
		const auto& urlUtf8 = urlStr.toUtf8 ();
		const QString& cinUrlStr = urlStr.toLower ();
		const auto& cinUrlUtf8 = cinUrlStr.toUtf8 ();

		const QString& domain = url.host ();
		const auto& domainUtf8 = domain.toUtf8 ();
		const bool isForeign = !req.rawHeader ("referer").contains (domainUtf8);

		QList<Filter> allFilters = Filters_;
		allFilters << UserFilters_->GetFilter ();
		Q_FOREACH (const Filter& filter, allFilters)
		{
			Q_FOREACH (const auto& item, filter.Exceptions_)
			{
				const auto& url = item.Option_.Case_ == Qt::CaseSensitive ? urlStr : cinUrlStr;
				const auto& utf8 = item.Option_.Case_ == Qt::CaseSensitive ? urlUtf8 : cinUrlUtf8;
				if (item.Option_.HideSelector_.isEmpty () && Matches (item, url, utf8, domain))
					return false;
			}

			Q_FOREACH (const auto& item, filter.Filters_)
			{
				if (!item.Option_.HideSelector_.isEmpty ())
					continue;

				const auto& opt = item.Option_;
				if (opt.AbortForeign_ && isForeign)
					continue;

				const auto& url = opt.Case_ == Qt::CaseSensitive ? urlStr : cinUrlStr;
				const auto& utf8 = opt.Case_ == Qt::CaseSensitive ? urlUtf8 : cinUrlUtf8;
				if (Matches (item, url, utf8, domain))
				{
					*matchedFilter = item.OrigString_;
					return true;
				}
			}
		}

		return false;
	}

	#if defined (Q_OS_WIN32) || defined (Q_OS_MAC)
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

	bool Core::Matches (const FilterItem& item, const QString& urlStr, const QByteArray& urlUtf8, const QString& domain) const
	{
		if (item.Option_.MatchObjects_ != FilterOption::MatchObject::All)
		{
			if (!(item.Option_.MatchObjects_ & FilterOption::MatchObject::CSS) &&
					!(item.Option_.MatchObjects_ & FilterOption::MatchObject::Image) &&
					!(item.Option_.MatchObjects_ & FilterOption::MatchObject::Script) &&
					!(item.Option_.MatchObjects_ & FilterOption::MatchObject::Object) &&
					!(item.Option_.MatchObjects_ & FilterOption::MatchObject::ObjSubrequest))
				return false;
		}

		const auto& opt = item.Option_;
		if (!opt.NotDomains_.isEmpty ())
		{
			Q_FOREACH (const auto& notDomain, opt.NotDomains_)
				if (domain.endsWith (notDomain, opt.Case_))
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

		switch (opt.MatchType_)
		{
		case FilterOption::MTRegexp:
			return item.RegExp_.Matches (urlStr);
		case FilterOption::MTWildcard:
			return WildcardMatches (item.OrigString_.constData (), urlUtf8.constData ());
		case FilterOption::MTPlain:
			return item.PlainMatcher_.indexIn (urlUtf8) >= 0;
		case FilterOption::MTBegin:
			return urlStr.startsWith (item.OrigString_);
		case FilterOption::MTEnd:
			return urlStr.endsWith (item.OrigString_);
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
				[] (const QString& t) { return t.trimmed (); });

		Filter f;
		std::for_each (lines.begin (), lines.end (), LineParser (&f));

		f.SD_.Filename_ = QFileInfo (filePath).fileName ();

		auto pos = std::find_if (Filters_.begin (), Filters_.end (),
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
		qDebug () << Q_FUNC_INFO << subscrUrl;
		QUrl url;
		if (subscrUrl.queryItemValue ("location").contains ("%"))
			url.setUrl (QUrl::fromPercentEncoding (subscrUrl.queryItemValue ("location").toAscii ()));
		else
			url.setUrl (subscrUrl.queryItemValue ("location"));
		QString subscrName = subscrUrl.queryItemValue ("title");

		if (Exists (subscrName) || Exists (url))
			return false;

		qDebug () << "adding" << url << "as" << subscrName;
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

		LeechCraft::Entity e =
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

	void Core::handleFrameLayout (QPointer<QWebFrame> frame)
	{
		const QUrl& url = frame->url ();
		const QString& urlStr = url.toString ();
		const auto& urlUtf8 = urlStr.toUtf8 ();
		const QString& cinUrlStr = urlStr.toLower ();
		const auto& cinUrlUtf8 = cinUrlStr.toUtf8 ();

		const QString& domain = url.host ();

		QList<Filter> allFilters = Filters_;
		allFilters << UserFilters_->GetFilter ();
		int numItems = 0;
		QList<QWebElement> elems;
		Q_FOREACH (const Filter& filter, allFilters)
			Q_FOREACH (const auto& item, filter.Filters_)
			{
				if (item.Option_.HideSelector_.isEmpty ())
					continue;

				const auto& opt = item.Option_;
				const auto& url = opt.Case_ == Qt::CaseSensitive ? urlStr : cinUrlStr;
				const auto& utf8 = opt.Case_ == Qt::CaseSensitive ? urlUtf8 : cinUrlUtf8;
				if (!item.OrigString_.isEmpty () && !Matches (item, url, utf8, domain))
					continue;

				Q_FOREACH (auto elem, frame->findAllElements (item.Option_.HideSelector_))
					RemoveElem (elem);

				if (!(++numItems % 100))
				{
					qApp->processEvents ();
					if (!frame)
					{
						qDebug () << Q_FUNC_INFO
								<< "frame destroyed in processEvents(), stopping";
						return;
					}
				}
			}
	}

	void Core::delayedRemoveElements (QPointer<QWebFrame> frame, const QString& url)
	{
		if (!frame)
			return;

		const auto& elems = frame->findAllElements ("*[src=\"" + url + "\"]");
		if (elems.count ())
			Q_FOREACH (QWebElement elem, elems)
				RemoveElem (elem);
		else if (frame->parentFrame ())
			delayedRemoveElements (frame->parentFrame (), url);
		else
		{
			connect (frame,
					SIGNAL (loadFinished (bool)),
					this,
					SLOT (moreDelayedRemoveElements ()),
					Qt::UniqueConnection);
			connect (frame,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleFrameDestroyed ()),
					Qt::UniqueConnection);
			MoreDelayedURLs_ [frame] << url;
		}
	}

	void Core::moreDelayedRemoveElements ()
	{
		QWebFrame *frame = qobject_cast<QWebFrame*> (sender ());
		Q_FOREACH (const QString& url, MoreDelayedURLs_ [frame])
		{
			QWebElementCollection elems =
					frame->findAllElements ("*[src=\"" + url + "\"]");
			if (elems.count ())
				Q_FOREACH (QWebElement elem, elems)
					elem.removeFromDocument ();
			else
				qWarning () << Q_FUNC_INFO << "not found" << url;
		}

		MoreDelayedURLs_.remove (frame);
	}

	void Core::handleFrameDestroyed ()
	{
		MoreDelayedURLs_.remove (static_cast<QWebFrame*> (sender ()));
	}
}
}
}
