#ifndef FILEPICKER_H
#define FILEPICKER_H
#include <QWidget>

class QLineEdit;
class QPushButton;

class FilePicker : public QWidget
{
	Q_OBJECT

	QLineEdit *LineEdit_;
	QPushButton *BrowseButton_;
public:
	FilePicker (QWidget *parent = 0);
	void SetText (const QString&);
	QString GetText () const;
private slots:
	void chooseFile ();
signals:
	void textChanged (const QString&);
};

#endif

