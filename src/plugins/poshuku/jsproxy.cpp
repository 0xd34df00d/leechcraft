#include "jsproxy.h"
#include <QString>
#include <QVariant>
#include <QtDebug>

JSProxy::JSProxy (QObject *parent)
: QObject (parent)
{
}

PageFormsData_t JSProxy::GetForms ()
{
	PageFormsData_t result = Current_;
	Current_.clear ();
	return result;
}

void JSProxy::setFormElement (const QString& url,
		int formId,
		const QString& elemName,
		const QString& elemType,
		const QVariant& value)
{
	ElementData ed =
	{
		elemName,
		elemType,
		value
	};
	Current_ [url] [formId] << ed;
}

