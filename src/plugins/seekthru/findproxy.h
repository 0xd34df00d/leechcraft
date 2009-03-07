#ifndef FINDPROXY_H
#define FINDPROXY_H
#include <QObject>
#include <interfaces/ifinder.h>
#include "searchhandler.h"

namespace LeechCraft
{
	namespace Util
	{
		class MergeModel;
	};
};

class FindProxy : public QObject
				, public IFindProxy
{
	Q_OBJECT
	Q_INTERFACES (IFindProxy);
	
	LeechCraft::Request R_;
	boost::shared_ptr<LeechCraft::Util::MergeModel> MergeModel_;
	QList<SearchHandler_ptr> Handlers_;
public:
	FindProxy (const LeechCraft::Request&);
	virtual ~FindProxy ();

	QAbstractItemModel* GetModel ();

	void SetHandlers (const QList<SearchHandler_ptr>&);
};

#endif

