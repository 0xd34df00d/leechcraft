#ifndef CRON_H
#define CRON_H
#include <QMainWindow>
#include <QMap>
#include <interfaces/interfaces.h>
#include "ui_mainwindow.h"

class Cron : public QMainWindow
		   , public IInfo
		   , public IWindow
		   , private Ui::MainWindow
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow);

	ID_t ID_;
	bool IsShown_;
	QMap<QString, QObject*> Providers_;
public:
	void Init ();
	QString GetName () const;
	QString GetInfo () const;
	QString GetStatusbarMessage () const;
	IInfo& SetID (ID_t);
	ID_t GetID () const;
	QStringList Provides () const;
	QStringList Needs () const;
	QStringList Uses () const;
	void SetProvider (QObject*, const QString&);
	void Release ();
	QIcon GetIcon () const;
	void SetParent (QWidget*);
	void ShowWindow ();
	void ShowBalloonTip ();
protected:
	virtual void closeEvent (QCloseEvent*);
};

#endif

