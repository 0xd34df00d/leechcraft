#include "passwordremember.h"
#include <QtDebug>
#include "core.h"

PasswordRemember::PasswordRemember (QWidget *parent)
: Notification (parent)
{
	Ui_.setupUi (this);
}

void PasswordRemember::add (const PageFormsData_t& data)
{
	bool init = false;

	QString url = data.keys ().at (0);
	ElementsData_t elems = data [url];
	ElementsData_t oldElems;
	Q_FOREACH (ElementData ed, elems)
	{
		if (ed.Type_.toLower () == "password" &&
				!ed.Value_.toString ().isEmpty ())
		{
			if (!init)
			{
				Core::Instance ().GetStorageBackend ()->
					GetFormsData (url, oldElems);
				init = true;
			}

			ElementsData_t::const_iterator pos =
				std::find_if (oldElems.begin (), oldElems.end (),
						ElemFinder (ed.Name_, ed.Type_));
			if (pos != oldElems.end () &&
					pos->Value_ == ed.Value_)
				continue;

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
	Core::Instance ().GetStorageBackend ()->
		SetFormsData (TempData_.first, TempData_.second);
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
	Core::Instance ().GetStorageBackend ()->
		SetFormsIgnored (TempData_.first, true);
	TempData_.first.clear ();
	TempData_.second.clear ();
	hide ();
}

