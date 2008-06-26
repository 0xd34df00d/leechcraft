#ifndef ADDMULTIPLETORRENTS_H
#define ADDMULTIPLETORRENTS_H
#include "ui_addmultipletorrents.h"

class AddMultipleTorrents : public QDialog, private Ui::AddMultipleTorrents
{
    Q_OBJECT
public:
    AddMultipleTorrents (QWidget *parent = 0);
    QString GetOpenDirectory () const;
    QString GetSaveDirectory () const;
	QLineEdit* GetEdit ();
	QStringList GetTags () const;
private slots:
    void on_BrowseOpen__released ();
    void on_BrowseSave__released ();
};

#endif

