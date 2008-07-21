#ifndef ADDJOB_H
#define ADDJOB_H
#include <QDialog>
#include "ui_addjob.h"

class AddJob : public QDialog
{
	Q_OBJECT

	Ui::AddJob Ui_;
	bool UserModifiedFilename_;
public:
	AddJob (QWidget* = 0);
	virtual ~AddJob ();

	struct Job
	{
		QString URL_;
		QString LocalPath_;
		QString Filename_;
		QString Comment_;

		Job (const QString&,
				const QString&,
				const QString&,
				const QString&);
	};

	Job GetJob () const;
public slots:
	virtual void accept ();
private slots:
	void on_URL__textEdited (const QString&);
	void on_LocalPath__textEdited ();
	void on_Filename__textEdited ();
	void on_BrowseButton__released ();
private:
	void CheckOK ();
};

#endif

