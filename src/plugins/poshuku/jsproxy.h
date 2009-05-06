#ifndef JSPROXY_H
#define JSPROXY_H
#include <QObject>

class JSProxy : public QObject
{
	Q_OBJECT
public:
	JSProxy (QObject* = 0);
public slots:
	/** Makes Core remember the form element.
	 */
	void setFormElement (const QString& url,
			const QString& formName,
			const QString& elemName,
			const QString& elemType,
			const QVariant& value);
};

#endif

