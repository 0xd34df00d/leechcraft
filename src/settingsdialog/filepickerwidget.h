#ifndef FILEPICKERWIDGET_H
#define FILEPICKERWIDGET_H
#include <QWidget>
#include <QtGui/QtGui>

class FilePickerWidget : public QWidget
{
	Q_OBJECT

	QPushButton *Browse_;
	QLineEdit *File_;
public:
	FilePickerWidget (QWidget *parent = 0);
	
	void setText (const QString&);
	QString text () const;
private slots:
	void initiateFileHandling ();
signals:
	void textChanged (const QString&);
};

#endif

