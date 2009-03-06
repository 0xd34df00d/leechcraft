#include "findproxy.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <plugininterface/mergemodel.h>
#include "searchhandler.h"

using LeechCraft::Util::MergeModel;

FindProxy::FindProxy (const LeechCraft::Request& r)
: MergeModel_ (new MergeModel (QStringList ("1") << "2" << "3"))
{
}

FindProxy::~FindProxy ()
{
	std::for_each
		(
			Handlers_.begin (), Handlers_.end (),
			boost::bind
				(
				boost::function<void (MergeModel*, QAbstractItemModel*)>
				(
					&MergeModel::RemoveModel
				),
				MergeModel_.get (),
				boost::bind
				(
					boost::function<SearchHandler* (boost::shared_ptr<SearchHandler>&)>
					(
						&boost::shared_ptr<SearchHandler>::get
					),
					_1
				)
			)
		);
}

QAbstractItemModel* FindProxy::GetModel ()
{
	return MergeModel_.get ();
}

