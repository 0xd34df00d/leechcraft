#ifndef IINFOADAPTOR_H
#define IINFOADAPTOR_H
#include <QDBusAbstractAdaptor>

class IInfo;
class QObject;

class IInfoAdaptor : public QDBusAbstractAdaptor
{
	Q_OBJECT

	IInfo *Object_;
public:
	IInfoAdaptor (QObject*);
};

#endif

