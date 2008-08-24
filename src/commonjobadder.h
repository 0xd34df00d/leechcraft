#ifndef COMMONJOBADDER_H
#define COMMONJOBADDER_H
#include <QDialog>
#include "ui_commonjobadder.h"

class CommonJobAdder : public QDialog, private Ui::CommonJobAdder
{
    Q_OBJECT
public:
    CommonJobAdder (QWidget *parent = 0);
	virtual ~CommonJobAdder ();
    QString GetString () const;
	QString GetWhere () const;
private slots:
    void on_Browse__released ();
	void on_BrowseWhere__released ();
    void on_Paste__released ();
};

#endif

