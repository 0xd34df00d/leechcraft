#ifndef FINDPROXY_H
#define FINDPROXY_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	namespace Util
	{
		class MergeModel;
	};
};

class SearchHandler;

class FindProxy : public QObject
				, public IFindProxy
{
	Q_OBJECT
	Q_INTERFACES (IFindProxy);

	boost::shared_ptr<LeechCraft::Util::MergeModel> MergeModel_;
	QList<boost::shared_ptr<SearchHandler> > Handlers_;
public:
	FindProxy (const LeechCraft::Request&);
	virtual ~FindProxy ();

	QAbstractItemModel* GetModel ();
};

#endif

