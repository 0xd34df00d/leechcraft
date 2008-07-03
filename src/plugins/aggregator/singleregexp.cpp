#include <QtDebug>
#include "singleregexp.h"

namespace
{
	inline bool IsRegexpValid (const QString& rx)
	{
		return QRegExp (rx).isValid () && !QRegExp (rx).isEmpty ();
	}
};

SingleRegexp::SingleRegexp (const QString& title,
		const QString& body,
		bool modifier,
		QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	connect (Ui_.TitleEdit_, SIGNAL (textChanged (const QString&)), this, SLOT (lineEdited (const QString&)));
	connect (Ui_.BodyEdit_, SIGNAL (textChanged (const QString&)), this, SLOT (lineEdited (const QString&)));

	Ui_.TitleEdit_->setText (title);
	Ui_.BodyEdit_->setText (body);

	if (modifier)
		Ui_.TitleEdit_->setEnabled (false);

	lineEdited (title, Ui_.TitleEdit_);
	lineEdited (body, Ui_.BodyEdit_);
}

QString SingleRegexp::GetTitle () const
{
	return Ui_.TitleEdit_->text ();
}

QString SingleRegexp::GetBody () const
{
	return Ui_.BodyEdit_->text ();
}

void SingleRegexp::lineEdited (const QString& newText, QWidget *setter)
{
	if (IsRegexpValid (newText))
		(setter ? setter : qobject_cast<QWidget*> (sender ()))
			->setStyleSheet ("background-color: rgba(0, 255, 0, 50);");
	else
		(setter ? setter : qobject_cast<QWidget*> (sender ()))
			->setStyleSheet ("background-color: rgba(255, 0, 0, 50);");
}

