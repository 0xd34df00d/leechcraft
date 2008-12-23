#ifndef INTERCEPTOR_H
#define INTERCEPTOR_H
#include <QObject>
#include <QMetaClassInfo>
#include "qtbrowserplugin.h"

class Interceptor : public QObject
				  , public QtNPBindable
{
	Q_OBJECT
	Q_CLASSINFO ("MIME", "application/x-bittorrent");
	
	QMap<QIODevice*, QByteArray> Data_;
public:
	Interceptor (QObject* = 0);
	virtual ~Interceptor ();

	virtual bool readData (QIODevice*, const QString&);
public slots:
	virtual void handleReadChannelFinished ();
};

#endif

