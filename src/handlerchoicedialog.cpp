#include "handlerchoicedialog.h"
#include <QRadioButton>
#include <interfaces/iinfo.h>

HandlerChoiceDialog::HandlerChoiceDialog (const QString& entity, QWidget *parent)
: QDialog (parent)
, Buttons_ (new QButtonGroup)
{
	Ui_.setupUi (this);
	Ui_.EntityLabel_->setText (entity);
	Ui_.DownloadersLabel_->hide ();
	Ui_.HandlersLabel_->hide ();
}

void HandlerChoiceDialog::Add (const IInfo *ii, IDownload *id)
{
	QRadioButton *but = new QRadioButton (ii->GetName (), this);
	but->setToolTip (ii->GetInfo ());

	if (!Buttons_->buttons ().size ())
		but->setChecked (true);

	Buttons_->addButton (but);
	Ui_.DownloadersLayout_->addWidget (but);
	Downloaders_ [ii->GetName ()] = id;

	Ui_.DownloadersLabel_->show ();
}

void HandlerChoiceDialog::Add (const IInfo *ii, IEntityHandler *ih)
{
	QRadioButton *but = new QRadioButton (ii->GetName (), this);
	but->setToolTip (ii->GetInfo ());

	if (!Buttons_->buttons ().size ())
		but->setChecked (true);

	Buttons_->addButton (but);
	Handlers_ [ii->GetName ()] = ih;
	Ui_.HandlersLayout_->addWidget (but);

	Ui_.HandlersLabel_->show ();
}

IDownload* HandlerChoiceDialog::GetDownload ()
{
	IDownload *result = 0;
	if (!Buttons_->checkedButton ())
		return 0;
	downloaders_t::iterator rit = Downloaders_.find (Buttons_->
			checkedButton ()->text ());
	if (rit != Downloaders_.end ())
		result = rit->second;
	return result;
}

IEntityHandler* HandlerChoiceDialog::GetEntityHandler ()
{
	IEntityHandler *result = 0;
	handlers_t::iterator rit = Handlers_.find (Buttons_->
			checkedButton ()->text ());
	if (rit != Handlers_.end ())
		result = rit->second;
	return result;
}

IEntityHandler* HandlerChoiceDialog::GetFirstEntityHandler ()
{
	if (Handlers_.size ())
		return Handlers_.begin ()->second;
	else
		return 0;
}

int HandlerChoiceDialog::NumChoices () const
{
	return Buttons_->buttons ().size ();
}

