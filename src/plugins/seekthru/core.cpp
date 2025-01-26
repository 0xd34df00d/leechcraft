/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <algorithm>
#include <stdexcept>
#include <QDomDocument>
#include <QMetaType>
#include <QFile>
#include <QSettings>
#include <QInputDialog>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <interfaces/idownload.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <util/sll/unreachable.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "findproxy.h"
#include "tagsasker.h"

namespace LC::SeekThru
{
	const QString Core::OS_ = "http://a9.com/-/spec/opensearch/1.1/";

	Core::Core ()
	{
		qRegisterMetaType<Description> ("LC::Plugins::SeekThru::Description");
#if QT_VERSION_MAJOR == 5
		qRegisterMetaTypeStreamOperators<UrlDescription> ("LC::Plugins::SeekThru::UrlDescription");
		qRegisterMetaTypeStreamOperators<QueryDescription> ("LC::Plugins::SeekThru::QueryDescription");
		qRegisterMetaTypeStreamOperators<Description> ("LC::Plugins::SeekThru::Description");
#endif

		ReadSettings ();
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::DoDelayedInit ()
	{
		Headers_ << tr ("Short name");
	}

	int Core::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant Core::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		if (index.column ())
			return {};

		Description d = Descriptions_.at (index.row ());
		switch (role)
		{
		case Qt::DisplayRole:
			return d.ShortName_;
		case RoleDescription:
			return d.Description_;
		case RoleContact:
			return d.Contact_;
		case RoleTags:
			return Proxy_->GetTagsManager ()->GetTags (d.Tags_);
		case RoleLongName:
			return d.LongName_;
		case RoleDeveloper:
			return d.Developer_;
		case RoleAttribution:
			return d.Attribution_;
		case RoleRight:
			switch (d.Right_)
			{
			case Description::SyndicationRight::Open:
				return tr ("Open");
			case Description::SyndicationRight::Limited:
				return tr ("Limited");
			case Description::SyndicationRight::Private:
				return tr ("Private");
			case Description::SyndicationRight::Closed:
				return tr ("Closed");
			}

			Util::Unreachable ();
		default:
			return {};
		}
	}

	Qt::ItemFlags Core::flags (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};
		else
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant Core::headerData (int pos, Qt::Orientation orient, int role) const
	{
		if (orient == Qt::Horizontal && role == Qt::DisplayRole)
			return Headers_.at (pos);
		else
			return QVariant ();
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

	int Core::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Descriptions_.size ();
	}

	void Core::SetProvider (QObject *provider, const QString& feature)
	{
		Providers_ [feature] = provider;
	}

	bool Core::CouldHandle (const Entity& e) const
	{
		if (e.Mime_ == "x-leechcraft/data-filter-request")
		{
			const auto& textVar = e.Entity_;
			if (!textVar.canConvert<QString> ())
				return false;

			if (e.Additional_.contains ("DataFilter"))
			{
				const auto& catVar = e.Additional_ ["DataFilter"].toByteArray ();
				const auto& catStr = QString::fromUtf8 (catVar.constData ());
				if (FindMatchingHRTag (catStr).isEmpty ())
					return false;
			}

			const auto& text = textVar.toString ().trimmed ().simplified ();
			return text.count ('\n') < 3 && text.size () < 200;
		}

		if (!e.Entity_.canConvert<QUrl> ())
			return false;

		QUrl url = e.Entity_.toUrl ();
		if (url.scheme () != "http" &&
				url.scheme () != "https")
			return false;

		if (e.Mime_ != "application/opensearchdescription+xml")
			return false;

		return true;
	}

	void Core::Handle (const Entity& e)
	{
		if (e.Mime_ != "x-leechcraft/data-filter-request")
		{
			Add (e.Entity_.toUrl ());
			return;
		}

		const auto& text = e.Entity_.toString ();
		const auto& catVar = e.Additional_ ["DataFilter"].toByteArray ();

		for (const auto& d : FindMatchingHRTag (QString::fromUtf8 (catVar.constData ())))
			for (const auto& u : d.URLs_)
			{
				if (!u.Type_.startsWith ("text/"))
					continue;

				const auto& url = u.MakeUrl (text, QHash<QString, QVariant> ());
				Proxy_->GetEntityManager ()->HandleEntity (Util::MakeEntity (url, {}, FromUserInitiated | OnlyHandle));
			}
	}

	void Core::Add (const QUrl& url)
	{
		auto name = Util::GetTemporaryName ();
		auto e = Util::MakeEntity (url, name,
				Internal |
					DoNotSaveInHistory |
					DoNotNotifyUser |
					NotPersistent);

		auto result = Proxy_->GetEntityManager ()->DelegateEntity (e);
		if (!result)
		{
			emit error (tr ("%1 wasn't delegated")
					.arg (url.toString ()));
			return;
		}

		Util::Sequence (this, result.DownloadResult_) >>
				Util::Visitor
				{
					[=, this] (const IDownload::Error&)
					{
						emit error (tr ("A job was delegated, but it failed.")
								.arg (name));
					},
					[=, this] (IDownload::Success)
					{
						QFile file { name };
						if (!file.open (QIODevice::ReadOnly))
						{
							emit error (tr ("Could not open file %1.")
									.arg (name));
							return;
						}

						HandleEntity (QString::fromUtf8 (file.readAll ()));

						if (!file.remove ())
							qWarning () << Q_FUNC_INFO
									<< "unable to remote temporary file:"
									<< name;
					}
				};
	}

	void Core::Remove (const QModelIndex& index)
	{
		QStringList oldCats = ComputeUniqueCategories ();

		beginRemoveRows (QModelIndex (), index.row (), index.row ());
		Descriptions_.removeAt (index.row ());
		endRemoveRows ();

		WriteSettings ();

		QStringList newCats = ComputeUniqueCategories ();
		emit categoriesChanged (newCats, oldCats);
	}

	void Core::SetTags (const QModelIndex& index, const QStringList& tags)
	{
		SetTags (index.row (), tags);
	}

	void Core::SetTags (int pos, const QStringList& tags)
	{
		if (pos >= Descriptions_.size () || pos < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid pos";
			return;
		}

		const auto oldCats = ComputeUniqueCategories ();

		Descriptions_ [pos].Tags_ = Proxy_->GetTagsManager ()->GetIDs (tags);

		WriteSettings ();

		QStringList newCats = ComputeUniqueCategories ();
		emit categoriesChanged (newCats, oldCats);
	}

	QStringList Core::GetCategories () const
	{
		QStringList result;
		for (const auto& descr : Descriptions_)
			result += Proxy_->GetTagsManager ()->GetTags (descr.Tags_);

		result.removeAll (QString ());
		result.sort ();
		result.erase (std::unique (result.begin (), result.end ()), result.end ());

		return result;
	}

	IFindProxy_ptr Core::GetProxy (const LC::Request& r)
	{
		QList<SearchHandler_ptr> handlers;
		for (const auto& d : Descriptions_)
		{
			const auto& ht = Proxy_->GetTagsManager ()->GetTags (d.Tags_);
			if (ht.contains (r.Category_))
			{
				auto sh = std::make_shared<SearchHandler> (d, Proxy_->GetEntityManager ());
				connect (sh.get (),
						SIGNAL (error (const QString&)),
						this,
						SIGNAL (error (const QString&)));
				handlers << sh;
			}
		}

		auto fp = std::make_shared<FindProxy> (r);
		fp->SetHandlers (handlers);
		return fp;
	}

	IWebBrowser* Core::GetWebBrowser () const
	{
		if (Providers_.contains ("webbrowser"))
			return qobject_cast<IWebBrowser*> (Providers_ ["webbrowser"]);
		else
			return 0;
	}

	void Core::HandleEntity (const QString& contents, const QString& useTags)
	{
		try
		{
			auto descr = ParseData (contents, useTags);

			const auto& oldCats = ComputeUniqueCategories ();

			beginInsertRows (QModelIndex (), Descriptions_.size (), Descriptions_.size ());
			Descriptions_ << descr;
			endInsertRows ();

			WriteSettings ();

			const auto& newCats = ComputeUniqueCategories ();
			emit categoriesChanged (newCats, oldCats);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
		}
	}

	QStringList Core::ComputeUniqueCategories () const
	{
		QSet<QString> ids;
		for (const auto& d : Descriptions_)
			for (const auto& id : d.Tags_)
				ids << id;

		return Proxy_->GetTagsManager ()->GetTags (ids.values ());
	}

	QList<Description> Core::FindMatchingHRTag (const QString& catStr) const
	{
		QList<Description> result;

		auto tm = Proxy_->GetTagsManager ();
		for (const auto& d : Descriptions_)
		{
			const auto matchingItem = std::find_if (d.Tags_.begin (), d.Tags_.end (),
					[&catStr, tm] (const QString& id) { return catStr == tm->GetTag (id); });
			if (matchingItem != d.Tags_.end ())
				result << d;
		}

		return result;
	}

	Description Core::ParseData (const QString& contents, const QString& useTags)
	{
		QDomDocument doc;
		QString errorString;
		int line, column;
		if (!doc.setContent (contents, true, &errorString, &line, &column))
		{
			qWarning () << contents;
			emit error (tr ("XML parse error %1 at %2:%3.")
					.arg (errorString)
					.arg (line)
					.arg (column));
			throw std::runtime_error ("Parse error");
		}

		auto root = doc.documentElement ();
		if (root.tagName () != "OpenSearchDescription")
			throw std::runtime_error ("Parse error");

		auto shortNameTag = root.firstChildElement ("ShortName");
		auto descriptionTag = root.firstChildElement ("Description");
		auto urlTag = root.firstChildElement ("Url");
		if (shortNameTag.isNull () ||
				descriptionTag.isNull () ||
				urlTag.isNull () ||
				!urlTag.hasAttribute ("template") ||
				!urlTag.hasAttribute ("type"))
			throw std::runtime_error ("Parse error");

		Description descr;
		descr.ShortName_ = shortNameTag.text ();
		descr.Description_ = descriptionTag.text ();

		while (!urlTag.isNull ())
		{
			UrlDescription d =
			{
				urlTag.attribute ("template"),
				urlTag.attribute ("type"),
				urlTag.attribute ("indexOffset", "1").toInt (),
				urlTag.attribute ("pageOffset", "1").toInt ()
			};
			descr.URLs_ << d;

			urlTag = urlTag.nextSiblingElement ("Url");
		}

		auto contactTag = root.firstChildElement ("Contact");
		if (!contactTag.isNull ())
			descr.Contact_ = contactTag.text ();

		if (useTags.isEmpty ())
		{
			QDomElement tagsTag = root.firstChildElement ("Tags");
			if (!tagsTag.isNull ())
				descr.Tags_ = Proxy_->GetTagsManager ()->Split (tagsTag.text ());
			else
				descr.Tags_ = QStringList ("default");

			TagsAsker ta (Proxy_->GetTagsManager ()->Join (descr.Tags_));
			QString userTags;
			qDebug () << Q_FUNC_INFO;
			if (ta.exec () == QDialog::Accepted)
				userTags = ta.GetTags ();

			if (!userTags.isEmpty ())
				descr.Tags_ = Proxy_->GetTagsManager ()->Split (userTags);
		}
		else
			descr.Tags_ = Proxy_->GetTagsManager ()->Split (useTags);

		descr.Tags_ = Proxy_->GetTagsManager ()->GetIDs (descr.Tags_);

		auto longNameTag = root.firstChildElement ("LongName");
		if (!longNameTag.isNull ())
			descr.LongName_ = longNameTag.text ();

		auto queryTag = root.firstChildElement ("Query");
		while (!queryTag.isNull () && queryTag.hasAttributeNS (OS_, "role"))
		{
			QueryDescription::Role r;
			QString role = queryTag.attributeNS (OS_, "role");
			if (role == "request")
				r = QueryDescription::RoleRequest;
			else if (role == "example")
				r = QueryDescription::RoleExample;
			else if (role == "related")
				r = QueryDescription::RoleRelated;
			else if (role == "correction")
				r = QueryDescription::RoleCorrection;
			else if (role == "subset")
				r = QueryDescription::RoleSubset;
			else if (role == "superset")
				r = QueryDescription::RoleSuperset;
			else
			{
				queryTag = queryTag.nextSiblingElement ("Query");
				continue;
			}

			QueryDescription d =
			{
				r,
				queryTag.attributeNS (OS_, "title"),
				queryTag.attributeNS (OS_, "totalResults", "-1").toInt (),
				queryTag.attributeNS (OS_, "searchTerms"),
				queryTag.attributeNS (OS_, "count", "-1").toInt (),
				queryTag.attributeNS (OS_, "startIndex", "-1").toInt (),
				queryTag.attributeNS (OS_, "startPage", "-1").toInt (),
				queryTag.attributeNS (OS_, "language", "*"),
				queryTag.attributeNS (OS_, "inputEncoding", "UTF-8"),
				queryTag.attributeNS (OS_, "outputEncoding", "UTF-8")
			};
			descr.Queries_ << d;
			queryTag = queryTag.nextSiblingElement ("Query");
		}

		auto developerTag = root.firstChildElement ("Developer");
		if (!developerTag.isNull ())
			descr.Developer_ = developerTag.text ();

		auto attributionTag = root.firstChildElement ("Attribution");
		if (!attributionTag.isNull ())
			descr.Attribution_ = attributionTag.text ();

		descr.Right_ = Description::SyndicationRight::Open;
		auto syndicationRightTag = root.firstChildElement ("SyndicationRight");
		if (!syndicationRightTag.isNull ())
		{
			auto sr = syndicationRightTag.text ();
			if (sr == "limited")
				descr.Right_ = Description::SyndicationRight::Limited;
			else if (sr == "private")
				descr.Right_ = Description::SyndicationRight::Private;
			else if (sr == "closed")
				descr.Right_ = Description::SyndicationRight::Closed;
		}

		descr.Adult_ = false;
		auto adultContentTag = root.firstChildElement ("AdultContent");
		if (!adultContentTag.isNull ())
		{
			auto text = adultContentTag.text ();
			if (!(text == "false" ||
					text == "FALSE" ||
					text == "0" ||
					text == "no" ||
					text == "NO"))
				descr.Adult_ = true;
		}

		auto languageTag = root.firstChildElement ("Language");
		bool was = false;;
		while (!languageTag.isNull ())
		{
			descr.Languages_ << languageTag.text ();
			was = true;
			languageTag = languageTag.nextSiblingElement ("Language");
		}
		if (!was)
			descr.Languages_ << "*";

		auto inputEncodingTag = root.firstChildElement ("InputEncoding");
		was = false;
		while (!inputEncodingTag.isNull ())
		{
			descr.InputEncodings_ << inputEncodingTag.text ();
			was = true;
			inputEncodingTag = inputEncodingTag.nextSiblingElement ("InputEncoding");
		}
		if (!was)
			descr.InputEncodings_ << "UTF-8";

		auto outputEncodingTag = root.firstChildElement ("OutputEncoding");
		was = false;
		while (!outputEncodingTag.isNull ())
		{
			descr.InputEncodings_ << outputEncodingTag.text ();
			was = true;
			outputEncodingTag = outputEncodingTag.nextSiblingElement ("OutputEncoding");
		}
		if (!was)
			descr.InputEncodings_ << "UTF-8";

		return descr;
	}

	void Core::ReadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SeekThru");
		int size = settings.beginReadArray ("Descriptions");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			Descriptions_ << settings.value ("Description").value<Description> ();
		}
		settings.endArray ();
	}

	void Core::WriteSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SeekThru");
		settings.beginWriteArray ("Descriptions");
		for (int i = 0; i < Descriptions_.size (); ++i)
		{
			settings.setArrayIndex (i);
			settings.setValue ("Description", QVariant::fromValue<Description> (Descriptions_.at (i)));
		}
		settings.endArray ();
	}

	bool Core::HandleDADescrAdded (QDataStream& in)
	{
		Description descr;
		in >> descr;
		if (in.status () != QDataStream::Ok)
		{
			qWarning () << Q_FUNC_INFO
					<< "bad stream status"
					<< in.status ();
			return false;
		}

		auto pos = std::find_if (Descriptions_.begin (), Descriptions_.end (),
				[descr] (const Description& d) { return d.ShortName_ == descr.ShortName_; });
		if (pos == Descriptions_.end ())
		{
			Descriptions_ << descr;
			SetTags (Descriptions_.size () - 1, descr.Tags_);
		}
		else
		{
			*pos = descr;
			SetTags (pos - Descriptions_.begin (), descr.Tags_);
		}
		return true;
	}

	bool Core::HandleDADescrRemoved (QDataStream& in)
	{
		QString shortName;
		in >> shortName;
		if (in.status () != QDataStream::Ok)
		{
			qWarning () << Q_FUNC_INFO
					<< "bad stream status"
					<< in.status ();
			return false;
		}

		auto pos = std::find_if (Descriptions_.begin (), Descriptions_.end (),
				[shortName] (const Description& d) { return d.ShortName_ == shortName; });
		if (pos != Descriptions_.end ())
			Descriptions_.erase (pos);
		return false;
	}

	bool Core::HandleDATagsChanged (QDataStream& in)
	{
		QString shortName;
		in >> shortName;
		QStringList tags;
		in >> tags;
		if (in.status () != QDataStream::Ok)
		{
			qWarning () << Q_FUNC_INFO
					<< "bad stream status"
					<< in.status ();
			return false;
		}

		auto pos = std::find_if (Descriptions_.begin (), Descriptions_.end (),
				[shortName] (const Description& d) { return d.ShortName_ == shortName; });
		if (pos != Descriptions_.end ())
		{
			SetTags (pos - Descriptions_.begin (), tags);
			return true;
		}
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "could not find the required description"
					<< shortName;
			return false;
		}
	}
}
