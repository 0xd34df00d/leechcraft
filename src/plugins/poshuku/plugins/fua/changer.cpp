#include "changer.h"
#include <QPushButton>

Changer::Changer (const QMap<QString, QString>& ids,
		const QString& suggestedDomain,
		const QString& selectedID,
		QWidget *parent)
: IDs_ (ids)
{
	Ui_.setupUi (this);

	Ui_.Agent_->addItems (ids.keys ());
	Ui_.Domain_->setText (suggestedDomain);
	Ui_.IDString_->setText (selectedID);
	Ui_.Agent_->setCurrentIndex (Ui_.Agent_->findText (IDs_.key (selectedID)));
	SetEnabled ();
}

QString Changer::GetDomain () const
{
	return Ui_.Domain_->text ();
}

QString Changer::GetID () const
{
	return Ui_.IDString_->text ();
}

void Changer::on_Domain__textChanged ()
{
	SetEnabled ();
}

void Changer::on_IDString__textChanged ()
{
	SetEnabled ();
}

void Changer::on_Agent__currentIndexChanged (const QString& agent)
{
	if (!agent.isEmpty ())
		Ui_.IDString_->setText (IDs_ [agent]);
}

void Changer::SetEnabled ()
{
	Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->
		setEnabled (Ui_.Domain_->text ().size () &&
				Ui_.IDString_->text ().size ());
}

