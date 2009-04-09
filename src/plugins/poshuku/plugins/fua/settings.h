#ifndef SETTINGS_H
#define SETTINGS_H
#include <QWidget>
#include "ui_settings.h"

class Poshuku_Fua;
class QStandardItemModel;

class Settings : public QWidget
{
	Q_OBJECT

	Ui::Settings Ui_;
	Poshuku_Fua *Fua_;
	QStandardItemModel *Model_;
public:
	Settings (QStandardItemModel*, Poshuku_Fua*);
private slots:
	void on_Add__released ();
	void on_Modify__released ();
	void on_Remove__released ();
};

#endif

