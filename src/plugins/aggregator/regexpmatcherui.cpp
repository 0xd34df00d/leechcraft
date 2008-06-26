#include <QMessageBox>
#include "singleregexp.h"
#include "regexpmatchermanager.h"
#include "regexpmatcherui.h"

RegexpMatcherUi::RegexpMatcherUi ()
{
	Ui_.setupUi (this);
}

RegexpMatcherUi::~RegexpMatcherUi ()
{
}

RegexpMatcherUi& RegexpMatcherUi::Instance ()
{
	static RegexpMatcherUi rmu;
	return rmu;
}

void RegexpMatcherUi::Release ()
{
}

void RegexpMatcherUi::on_AddRegexpButton__released ()
{
	bool success = false;
	QString title, body;
	do
	{
		success = true;
		SingleRegexp srx (title, body, this);
		if (srx.exec () == QDialog::Rejected)
			return;

		title = srx.GetTitle ();
		body = srx.GetBody ();

		if (title.isEmpty () || body.isEmpty ())
		{
			QMessageBox::warning (this, tr ("Warning"), tr ("Title matcher or body extractor could not "
						"be empty."));
			success = false;
			continue;
		}

		try
		{
			RegexpMatcherManager::Instance ().Add (title, body);
		}
		catch (const RegexpMatcherManager::AlreadyExists&)
		{
			QMessageBox::warning (this, tr ("Warning"), tr ("This title matcher regexp already exists. "
						"Specify another one or modify existing title matcher regexp's body extractor."));
			success = false;
		}
	}
	while (!success);
}

void RegexpMatcherUi::on_ModifyRegexpButton__released ()
{
	bool success = false;
}

