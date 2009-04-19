#ifndef PLUGINS_POSHUKU_BACKENDSELECTOR_H
#define PLUGINS_POSHUKU_BACKENDSELECTOR_H
#include <QWidget>
#include "ui_backendselector.h"

class BackendSelector : public QWidget
{
	Q_OBJECT

	Ui::BackendSelector Ui_;
public:
	BackendSelector (QWidget* = 0);
private:
	void FillUI ();
public slots:
	void accept ();
	void reject ();
};

#endif

