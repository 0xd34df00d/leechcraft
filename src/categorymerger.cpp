#include "categorymerger.h"
#include "core.h"
#include "requestparser.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

CategoryMerger::CategoryMerger (QObject *parent)
: QObject (parent)
, Model_ (new MergeModel (QStringList (tr ("Entity"))
			<< tr ("Category")
			<< tr ("Information")))
{
}

void CategoryMerger::SetRequest (const Request& request)
{
	for (proxies_t::const_iterator i = Proxies_.begin (),
			end = Proxies_.end (); i != end; ++i)
		Model_->RemoveModel ((*i)->GetModel ());
	Proxies_.clear ();

	QList<IFinder*> finders = Core::Instance ().GetPluginManager ()->
		GetAllCastableTo<IFinder*> ();

	for (QList<IFinder*>::iterator i = finders.begin (),
			end = finders.end (); i != end; ++i)
	{
		if (!(*i)->GetCategories ().contains (request.Category_))
			continue;

		boost::shared_ptr<IFindProxy> proxy = (*i)->GetProxy (request);
		Model_->AddModel (proxy->GetModel ());
		Proxies_.push_back (proxy);
	}
}

Util::MergeModel* CategoryMerger::GetModel () const
{
	return Model_.get ();
}

