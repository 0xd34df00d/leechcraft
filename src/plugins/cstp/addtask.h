#ifndef ADDTASK_H
#define ADDTASK_H
#include <QDialog>
#include "ui_addtask.h"

class AddTask : public QDialog
{
	Q_OBJECT

	Ui::AddTask Ui_;
	bool UserModifiedFilename_;
public:
	AddTask (QWidget* = 0);
	AddTask (const QString&, const QString&, QWidget* = 0);
	virtual ~AddTask ();

	struct Task
	{
		QString URL_;
		QString LocalPath_;
		QString Filename_;
		QString Comment_;

		Task (const QString&,
				const QString&,
				const QString&,
				const QString&);
	};

	Task GetTask () const;
public slots:
	virtual void accept ();
private slots:
	void on_URL__textEdited (const QString&);
	void on_LocalPath__textChanged ();
	void on_Filename__textEdited ();
	void on_BrowseButton__released ();
private:
	void CheckOK ();
};

#endif

