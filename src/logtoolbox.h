#ifndef LOGTOOLBOX_H
#define LOGTOOLBOX_H
#include "ui_logtoolbox.h"

class LogToolBox : public QDialog
{
	Q_OBJECT
	
	Ui::LogToolBox Ui_;
public:
	LogToolBox (QWidget* = 0);
	virtual ~LogToolBox ();
public slots:
	void log (const QString&);
private slots:
	void on_Clear__released ();
};

#endif

