#ifndef FINDPROXY_H
#define FINDPROXY_H
#include <QObject>
#include <interfaces/ifinder.h>

class FindProxy : public QObject
				, public IFindProxy
{
	Q_OBJECT
	Q_INTERFACES (IFindProxy);

	QByteArray ID_;
public:
	FindProxy (const LeechCraft::Request&, QObject* = 0);
	virtual ~FindProxy ();
};

#endif

