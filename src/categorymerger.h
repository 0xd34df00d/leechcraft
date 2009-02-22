#ifndef CATEGORYMERGER_H
#define CATEGORYMERGER_H
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <plugininterface/mergemodel.h>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	struct Request;

	class CategoryMerger : public QObject
	{
		Q_OBJECT

		std::auto_ptr<Util::MergeModel> Model_;
		typedef std::vector<boost::shared_ptr<IFindProxy> > proxies_t;
		proxies_t Proxies_;
	public:
		CategoryMerger (QObject* = 0);

		void SetRequest (const Request&);
		Util::MergeModel* GetModel () const;
	};
};

#endif

