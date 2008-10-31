#ifndef TABWIDGET_H
#define TABWIDGET_H
#include <QWidget>
#include "ui_tabwidget.h"

class TabWidget : public QWidget
{
	Q_OBJECT
	
	Ui::TabWidget Ui_;
public:
	TabWidget (QWidget* = 0);
	Phonon::VideoWidget* GetVideoOutput () const;
};

#endif

