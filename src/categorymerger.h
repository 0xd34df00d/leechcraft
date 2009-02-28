#ifndef CATEGORYMERGER_H
#define CATEGORYMERGER_H
#include <vector>
#include <boost/shared_ptr.hpp>
#include <plugininterface/mergemodel.h>
#include <interfaces/ifinder.h>
#include "filtermodel.h"

namespace LeechCraft
{
	struct Request;

	class CategoryMerger : public Util::MergeModel
	{
		Q_OBJECT

		typedef std::vector<boost::shared_ptr<IFindProxy> > proxies_t;
		proxies_t Proxies_;
		boost::shared_ptr<Util::MergeModel> MergeModel_;
		boost::shared_ptr<Util::MergeModel> HistoryMergeModel_;
		std::auto_ptr<FilterModel> FilterModel_;
	public:
		CategoryMerger (const Request&,
				const boost::shared_ptr<Util::MergeModel>&,
				const boost::shared_ptr<Util::MergeModel>&,
				QObject* = 0);
	};
};

#endif

