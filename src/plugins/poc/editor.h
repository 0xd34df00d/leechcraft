#ifndef EDITOR_H
#define EDITOR_H

#include "ui_poc.h"

class Editor : public QWidget, Ui::PoCWidget
{
	public:
		Editor(QWidget * parent = 0);
};

#endif