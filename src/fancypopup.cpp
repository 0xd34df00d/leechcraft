#include "fancypopup.h"
#include <QLabel>
#include <QVBoxLayout>

Main::FancyPopup::FancyPopup (const QString& title,
		const QString& text)
: QFrame (0, Qt::ToolTip | Qt::WindowStaysOnTopHint)
{
	Ui_.setupUi (this);

	QWidget::setAttribute (Qt::WA_DeleteOnClose);
	setWindowModality (Qt::NonModal);

	Ui_.Title_->setText (title);
	Ui_.Text_->setText (text);

	Ui_.Title_->setAutoFillBackground (true);
}

Main::FancyPopup::~FancyPopup ()
{
}

void Main::FancyPopup::mouseReleaseEvent (QMouseEvent*)
{
	emit clicked ();
}

