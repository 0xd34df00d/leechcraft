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

	PageFormsData_t GetForms () const;
	void SetForms (const PageFormsData_t&);
	void ClearForms ();
public slots:
	void debug (const QString& str);
	void warning (const QString& str);

	/** Makes Core remember the form element.
	 */
	void setFormElement (const QString& url,
			int formId,
			const QString& elemName,
			const QString& elemType,
			const QVariant& value);
	
	/** @brief Returns the stored element's value (or empty QVariant if
	 * none).
	 *
	 * If there is only one element with given elemName, then it's
	 * returned regardless formId, otherwise formId is taken into
	 * account.
	 */
	QVariant getFormElement (int formId,
			const QString& elemName,
			const QString& elemType) const;
};

#endif

