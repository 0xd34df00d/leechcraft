#include "passwordremember.h"

PasswordRemember::PasswordRemember (QWidget *parent)
: Notification (parent)
{
	Ui_.setupUi (this);
}

void PasswordRemember::add (const PageFormsData_t& data)
{
	QString url = data.keys ().at (0);
	ElementsData_t elems = data [url];
	Q_FOREACH (ElementData ed, elems)
	{
		if (ed.Type_.toLower () == "password" &&
				!ed.Value_.toString ().isEmpty ())
		{
			// If there is already some data awaiting for user
			// response, don't add new one.
			if (TempData_.first.size ())
				return;

			TempData_ = qMakePair (url, elems);

			show ();
		}
	}
}

void PasswordRemember::on_Remember__released ()
{
	Core::Instance ().GetStorageBackend ()->SetFormsData (TempData_.first,
			TempData_.second);
	hide ();
}

void PasswordRemember::on_NotNow__released ()
{
	TempData_.first.clear ();
	TempData_.second.clear ();
	hide ();
}

void PasswordRemember::on_Never__released ()
{
	// TODO implement never case
	TempData_.first.clear ();
	TempData_.second.clear ();
	hide ();
}

