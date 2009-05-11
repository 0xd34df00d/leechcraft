#include "passwordremember.h"

PasswordRemember::PasswordRemember (QWidget *parent)
: Notification (parent)
{
	Ui_.setupUi (this);
}

void PasswordRemember::add (const PageFormsData_t& data)
{
	FormsData_t forms = data.values ().at (0);
	QList<int> indexes = forms.keys ();
	Q_FOREACH (int index, indexes)
	{
		ElementsData_t elements = forms [index];
		Q_FOREACH (ElementData ed, elements)
		{
			if (ed.Type_.toLower () == "password" &&
					!ed.Value_.toString ().isEmpty ())
			{
				// If there is already some data awaiting for user
				// response, don't add new one.
				if (TempData_.size ())
					return;

				TempData_ = elements;

				show ();
			}
		}
	}
}

void PasswordRemember::on_Remember__released ()
{
	hide ();
}

void PasswordRemember::on_NotNow__released ()
{
	hide ();
}

void PasswordRemember::on_Never__released ()
{
	hide ();
}

