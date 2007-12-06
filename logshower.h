#ifndef LOGSHOWER_H
#define LOGSHOWER_H
#include <QListWidget>

class LogShower : public QListWidget
{
	Q_OBJECT
public:
	LogShower (QWidget *parent = 0);
	void AddDownloadMessage (const QString&);
	void AddUploadMessage (const QString&);
};

#endif

