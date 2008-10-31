#include "tabwidget.h"

TabWidget::TabWidget (QWidget *parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
}

Phonon::VideoWidget* TabWidget::GetVideoOutput () const
{
	return Ui_.VideoWidget_;
}

