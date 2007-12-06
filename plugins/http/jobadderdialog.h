#ifndef JOBADDERDIALOG_H
#define JOBADDERDIALOG_H
#include <QDialog>

class QPushButton;
class QLineEdit;
class QLabel;
class JobParams;
class QCheckBox;

class JobAdderDialog : public QDialog
{
	Q_OBJECT

	QLabel *URLLabel_, *LocalLabel_;
	QLineEdit *URL_, *LocalName_;
	QCheckBox *Autostart_;

	QPushButton *OK_, *Cancel_;
public:
	JobAdderDialog (QWidget *parent = 0);
public slots:
	virtual void done (int r);
private slots:
	void selectDir ();
signals:
	void gotParams (JobParams*);
};

#endif

