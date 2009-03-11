#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <QDomDocument>
#include <QMetaType>
#include <QFile>
#include <QSettings>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
#include "findproxy.h"

using LeechCraft::Util::Proxy;

const QString Core::OS_ = "http://a9.com/-/spec/opensearch/1.1/";

Core::Core ()
: Headers_ (tr ("Short name"))
{
	qRegisterMetaType<Description> ("Description");
	qRegisterMetaTypeStreamOperators<UrlDescription> ("UrlDescription");
	qRegisterMetaTypeStreamOperators<QueryDescription> ("QueryDescription");
	qRegisterMetaTypeStreamOperators<Description> ("Description");
	ReadSettings ();

	GetCategories ();
}

Core& Core::Instance ()
{
	static Core c;
	return c;
}

int Core::columnCount (const QModelIndex&) const
{
	return Headers_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	if (!index.isValid ())
		return QVariant ();

	Description d = Descriptions_.at (index.row ());
	switch (index.column ())
	{
		case 0:
			switch (role)
			{
				case Qt::DisplayRole:
					return d.ShortName_;
				case RoleDescription:
					return d.Description_;
				case RoleContact:
					return d.Contact_;
				case RoleTags:
					return d.Tags_;
				case RoleLongName:
					return d.LongName_;
				case RoleDeveloper:
					return d.Developer_;
				case RoleAttribution:
					return d.Attribution_;
				case RoleRight:
					switch (d.Right_)
					{
						case Description::SROpen:
							return tr ("Open");
						case Description::SRLimited:
							return tr ("Limited");
						case Description::SRPrivate:
							return tr ("Private");
						case Description::SRClosed:
							return tr ("Closed");
					}
				default:
					return QVariant ();
			}
		default:
			return QVariant ();
	}
}

Qt::ItemFlags Core::flags (const QModelIndex& index) const
{
	if (!index.isValid ())
		return 0;
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

void Core::Add (const QString& url)
{
	QString name = LeechCraft::Util::GetTemporaryName ();
	LeechCraft::DownloadEntity e =
	{
		url.toUtf8 (),
		name,
		QString (),
		LeechCraft::Internal |
			LeechCraft::DoNotSaveInHistory |
			LeechCraft::DoNotNotifyUser |
			LeechCraft::NotPersistent
	};

	int id = -1;
	QObject *provider;
	emit delegateEntity (e, &id, &provider);
	if (id == -1)
	{
		emit error (tr ("%1 wasn't delegated")
				.arg (url));
		return;
	}

	HandleProvider (provider);
	Jobs_ [id] = name;
}

void Core::Remove (const QModelIndex& index)
{
	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	Descriptions_.removeAt (index.row ());
	endRemoveRows ();
	WriteSettings ();
}

QStringList Core::GetCategories () const
{
	QStringList result;
	for (QList<Description>::const_iterator i = Descriptions_.begin (),
			end = Descriptions_.end (); i != end; ++i)
		result += i->Tags_;

	result.sort ();
	result.erase (std::unique (result.begin (), result.end ()), result.end ());

	return result;
}

IFindProxy_ptr Core::GetProxy (const LeechCraft::Request& r)
{
	QList<SearchHandler_ptr> handlers;
	Q_FOREACH (Description d, Descriptions_)
		if (d.Tags_.contains (r.Category_))
		{
			SearchHandler_ptr sh (new SearchHandler (d));
			connect (sh.get (),
					SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
							int*, QObject**)),
					this,
					SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
							int*, QObject**)));
			connect (sh.get (),
					SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
					this,
					SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
			connect (sh.get (),
					SIGNAL (error (const QString&)),
					this,
					SIGNAL (error (const QString&)));
			connect (sh.get (),
					SIGNAL (warning (const QString&)),
					this,
					SIGNAL (warning (const QString&)));
			handlers << sh;
		}

	boost::shared_ptr<FindProxy> fp (new FindProxy (r));
	fp->SetHandlers (handlers);
	return IFindProxy_ptr (fp);
}

IWebBrowser* Core::GetWebBrowser () const
{
	if (Providers_.contains ("webbrowser"))
		return qobject_cast<IWebBrowser*> (Providers_ ["webbrowser"]);
	else
		return 0;
}

void Core::handleJobFinished (int id)
{
	if (!Jobs_.contains (id))
		return;
	QString filename = Jobs_ [id];
	Jobs_.remove (id);

	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		emit error (tr ("Could not open file %1.")
				.arg (filename));
		return;
	}

	HandleEntity (file.readAll ());

	file.close ();
	if (!file.remove ())
		emit warning (tr ("Could not remove temporary file %1.")
				.arg (filename));
}

void Core::handleJobError (int id)
{
	if (!Jobs_.contains (id))
		return;
	emit error (tr ("A job was delegated, but it failed.")
			.arg (Jobs_ [id]));
	Jobs_.remove (id);
}

void Core::HandleEntity (const QString& contents)
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
		return;
	}

	QDomElement root = doc.documentElement ();
	if (root.tagName () != "OpenSearchDescription")
	{
		emit error (tr ("Not and OpenSearch description."));
		return;
	}

	QDomElement shortNameTag = root.firstChildElement ("ShortName");
	QDomElement descriptionTag = root.firstChildElement ("Description");
	QDomElement urlTag = root.firstChildElement ("Url");
	if (shortNameTag.isNull () ||
			descriptionTag.isNull () ||
			urlTag.isNull () ||
			!urlTag.hasAttribute ("template") ||
			!urlTag.hasAttribute ("type"))
	{
		emit error (tr ("Malformed OpenSearch description."));
		return;
	}

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

	QDomElement contactTag = root.firstChildElement ("Contact");
	if (!contactTag.isNull ())
		descr.Contact_ = contactTag.text ();

	QDomElement tagsTag = root.firstChildElement ("Tags");
	if (!tagsTag.isNull ())
		descr.Tags_ = tagsTag.text ().split (' ', QString::SkipEmptyParts);
	else
		descr.Tags_ = QStringList ("default");

	QDomElement longNameTag = root.firstChildElement ("LongName");
	if (!longNameTag.isNull ())
		descr.LongName_ = longNameTag.text ();

	QDomElement queryTag = root.firstChildElement ("Query");
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

	QDomElement developerTag = root.firstChildElement ("Developer");
	if (!developerTag.isNull ())
		descr.Developer_ = developerTag.text ();

	QDomElement attributionTag = root.firstChildElement ("Attribution");
	if (!attributionTag.isNull ())
		descr.Attribution_ = attributionTag.text ();

	descr.Right_ = Description::SROpen;
	QDomElement syndicationRightTag = root.firstChildElement ("SyndicationRight");
	if (!syndicationRightTag.isNull ())
	{
		QString sr = syndicationRightTag.text ();
		if (sr == "limited")
			descr.Right_ = Description::SRLimited;
		else if (sr == "private")
			descr.Right_ = Description::SRPrivate;
		else if (sr == "closed")
			descr.Right_ = Description::SRClosed;
	}

	descr.Adult_ = false;
	QDomElement adultContentTag = root.firstChildElement ("AdultContent");
	if (!adultContentTag.isNull ())
	{
		QString text = adultContentTag.text ();
		if (!(text == "false" ||
				text == "FALSE" ||
				text == "0" ||
				text == "no" ||
				text == "NO"))
			descr.Adult_ = true;
	}

	QDomElement languageTag = root.firstChildElement ("Language");
	bool was = false;;
	while (!languageTag.isNull ())
	{
		descr.Languages_ << languageTag.text ();
		was = true;
		languageTag = languageTag.nextSiblingElement ("Language");
	}
	if (!was)
		descr.Languages_ << "*";

	QDomElement inputEncodingTag = root.firstChildElement ("InputEncoding");
	was = false;
	while (!inputEncodingTag.isNull ())
	{
		descr.InputEncodings_ << inputEncodingTag.text ();
		was = true;
		inputEncodingTag = inputEncodingTag.nextSiblingElement ("InputEncoding");
	}
	if (!was)
		descr.InputEncodings_ << "UTF-8";

	QDomElement outputEncodingTag = root.firstChildElement ("OutputEncoding");
	was = false;
	while (!outputEncodingTag.isNull ())
	{
		descr.InputEncodings_ << outputEncodingTag.text ();
		was = true;
		outputEncodingTag = outputEncodingTag.nextSiblingElement ("OutputEncoding");
	}
	if (!was)
		descr.InputEncodings_ << "UTF-8";

	beginInsertRows (QModelIndex (), Descriptions_.size (), Descriptions_.size ());
	Descriptions_ << descr;
	endInsertRows ();

	WriteSettings ();
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
			SLOT (handleJobError (int)));
}

void Core::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_SeekThru");
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
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_SeekThru");
	settings.beginWriteArray ("Descriptions");
	for (int i = 0; i < Descriptions_.size (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("Description",
				QVariant::fromValue<Description> (Descriptions_.at (i)));
	}
	settings.endArray ();
}

