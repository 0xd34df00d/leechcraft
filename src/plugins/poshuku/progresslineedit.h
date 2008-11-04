#ifndef PROGRESSLINEEDIT_H
#define PROGRESSLINEEDIT_H
#include <QLineEdit>

class ProgressLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	ProgressLineEdit (QWidget* = 0);
	virtual ~ProgressLineEdit ();
public slots:
	void setValue (int);
};

#endif

