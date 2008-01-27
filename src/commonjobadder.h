#ifndef COMMONJOBADDER_H
#define COMMONJOBADDER_H
#include <QDialog>
#include "ui_commonjobadder.h"

class CommonJobAdder : public QDialog, private Ui::CommonJobAdder
{
	Q_OBJECT
public:
	CommonJobAdder (QWidget *parent = 0);
	QString GetString () const;
};

#endif

