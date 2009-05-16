#include "categorymerger.h"
#include <QtDebug>
#include "core.h"
#include "requestparser.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

CategoryMerger::CategoryMerger (const Request& r,
		const boost::shared_ptr<MergeModel>& merge,
		QObject *parent)
: MergeModel (QStringList (tr ("Name"))
		<< tr ("State")
		<< tr ("Progress"))
, MergeModel_ (merge)
, FilterModel_ (new FilterModel)
{
	setObjectName (QString ("CategoryMerger ") + r.String_);
	FilterModel_->setObjectName (QString ("CategoryMerger's filter model ") + r.String_);
	setProperty ("__LeechCraft_own_core_model", true);
	FilterModel_->setProperty ("__LeechCraft_own_core_model", true);
	bool builtin = false;

	if (r.Category_.isEmpty () ||
			r.Category_ == "downloads" ||
			r.Category_ == "d")
	{
		builtin = true;
		FilterModel_->setSourceModel (MergeModel_.get ());
	}
	else
	{
		QList<IFinder*> finders = Core::Instance ().GetPluginManager ()->
			GetAllCastableTo<IFinder*> ();

		for (QList<IFinder*>::iterator i = finders.begin (),
				end = finders.end (); i != end; ++i)
		{
			if (!(*i)->GetCategories ().contains (r.Category_))
				continue;

			boost::shared_ptr<IFindProxy> proxy = (*i)->GetProxy (r);
			AddModel (proxy->GetModel ());
			Proxies_.push_back (proxy);
		}
	}

	if (builtin)
	{
		FilterModel_->setFilterCaseSensitivity (r.CaseSensitive_ ?
				Qt::CaseSensitive : Qt::CaseInsensitive);

		switch (r.Type_)
		{
			case Request::RTFixed:
				FilterModel_->SetTagsMode (false);
				FilterModel_->setFilterFixedString (r.String_);
				break;
			case Request::RTWildcard:
				FilterModel_->SetTagsMode (false);
				FilterModel_->setFilterWildcard (r.String_);
				break;
			case Request::RTRegexp:
				FilterModel_->SetTagsMode (false);
				FilterModel_->setFilterRegExp (r.String_);
				break;
			case Request::RTTag:
				FilterModel_->SetTagsMode (true);
				FilterModel_->setFilterFixedString (r.String_);
				break;
		}
		AddModel (FilterModel_.get ());
	}
}

