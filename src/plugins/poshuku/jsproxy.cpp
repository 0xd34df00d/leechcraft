#include "jsproxy.h"
#include <QString>
#include <QVariant>
#include <QtDebug>

JSProxy::JSProxy (QObject *parent)
: QObject (parent)
{
}

void JSProxy::setFormElement (const QString& url,
		const QString& formName,
		const QString& elemName,
		const QString& elemType,
		const QVariant& value)
{
	qDebug () << url << formName << elemName << elemType << value;
}

