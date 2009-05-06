#ifndef JSPROXY_H
#define JSPROXY_H
#include <QObject>
#include <QVariant>

struct ElementData
{
	QString Name_;
	QString Type_;
	QVariant Value_;
};

typedef QList<ElementData> ElementsData_t;
typedef QMap<int, ElementsData_t> FormsData_t;
typedef QMap<QString, FormsData_t> PageFormsData_t;

class JSProxy : public QObject
{
	Q_OBJECT

	PageFormsData_t Current_;
public:
	JSProxy (QObject* = 0);

	PageFormsData_t GetForms ();
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

