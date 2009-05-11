#ifndef JSPROXY_H
#define JSPROXY_H
#include <QObject>
#include <QVariant>
#include "customwebpage.h"

class JSProxy : public QObject
{
	Q_OBJECT

	PageFormsData_t Current_;
public:
	JSProxy (QObject* = 0);

	PageFormsData_t GetForms ();
	void ClearForms ();
public slots:
	/** Makes Core remember the form element.
	 */
	void setFormElement (const QString& url,
			int formId,
			const QString& elemName,
			const QString& elemType,
			const QVariant& value);
};

#endif

