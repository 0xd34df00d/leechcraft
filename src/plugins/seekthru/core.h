#ifndef CORE_H
#define CORE_H
#include <QObject>
#include <QMap>
#include <interfaces/structures.h>
#include "description.h"

class Core : public QObject
{
	Q_OBJECT

	QObjectList Downloaders_;
	QMap<int, QString> Jobs_;

	static const QString OS_;

	Core ();
public:
	static Core& Instance ();
	/** Fetches the searcher from the url.
	 */
	void Add (const QString& url);
private slots:
	void handleJobFinished (int);
	void handleJobRemoved (int);
	void handleJobError (int);
private:
	void HandleEntity (const QString&);
	void HandleProvider (QObject*);
signals:
	void error (const QString&);
	void warning (const QString&);
	void delegateEntity (const LeechCraft::DownloadEntity&,
			int*, QObject**);
};

#endif

