#include "progresslineedit.h"

ProgressLineEdit::ProgressLineEdit (QWidget *parent)
: QLineEdit (parent)
{
	setValue (0);
}

ProgressLineEdit::~ProgressLineEdit ()
{
}

void ProgressLineEdit::setValue (int value)
{
	double sv = static_cast<double> (value) / 100;
	QString ss = QString ("QLineEdit { "
			"background-color: qlineargradient(spread:pad, "
			"x1:0, y1:1, x2:1, y2:1, "
			"stop:%1 rgba(160, 160, 160, 200), "
			"stop:%2 rgba(255, 255, 255, 255)) }")
		.arg (sv)
		.arg (sv + 0.05);

	setStyleSheet (ss);
}

