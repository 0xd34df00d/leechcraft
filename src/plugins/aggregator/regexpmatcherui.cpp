#include <QMessageBox>
#include <QHeaderView>
#include "singleregexp.h"
#include "regexpmatchermanager.h"
#include "regexpmatcherui.h"

RegexpMatcherUi::RegexpMatcherUi ()
{
	Ui_.setupUi (this);
	Ui_.Regexps_->setModel (&RegexpMatcherManager::Instance ());
}

RegexpMatcherUi::~RegexpMatcherUi ()
{
	RegexpMatcherManager::Instance ().Release ();
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
		SingleRegexp srx (title, body, false, this);
		if (srx.exec () == QDialog::Rejected)
			return;

		title = srx.GetTitle ();
		body = srx.GetBody ();

		try
		{
			RegexpMatcherManager::Instance ().Add (title, body);
		}
		catch (const RegexpMatcherManager::AlreadyExists&)
		{
			QMessageBox::warning (this, tr ("Warning"), tr ("This title "
						"matcher regexp already exists. Specify another "
						"one or modify existing title matcher regexp's "
						"body extractor."));
			success = false;
		}
		catch (const RegexpMatcherManager::Malformed&)
		{
			QMessageBox::warning (this, tr ("Warning"), tr ("Either title"
						" matcher or body extractor is malformed."));
			success = false;
		}
	}
	while (!success);
}

void RegexpMatcherUi::on_ModifyRegexpButton__released ()
{
	QModelIndex index = Ui_.Regexps_->selectionModel ()->currentIndex ();
	if (!index.isValid ())
		return;

	bool success = false;
	RegexpMatcherManager::titlebody_t pair = RegexpMatcherManager::Instance ().GetTitleBody (index);
	QString title = pair.first,
			body = pair.second;
	do
	{
		success = true;
		SingleRegexp srx (title, body, true, this);
		if (srx.exec () == QDialog::Rejected)
			return;

		body = srx.GetBody ();

		try
		{
			RegexpMatcherManager::Instance ().Modify (title, body);
		}
		catch (const RegexpMatcherManager::AlreadyExists&)
		{
			QMessageBox::warning (this, tr ("Warning"), tr ("This title "
						"matcher regexp already exists. Specify another "
						"one or modify existing title matcher regexp's "
						"body extractor."));
			success = false;
		}
		catch (const RegexpMatcherManager::Malformed&)
		{
			QMessageBox::warning (this, tr ("Warning"), tr ("Either title"
						" matcher or body extractor is malformed."));
			success = false;
		}
	}
	while (!success);
}

void RegexpMatcherUi::on_RemoveRegexpButton__released ()
{
	QModelIndex index = Ui_.Regexps_->selectionModel ()->currentIndex ();
	if (!index.isValid ())
		return;

	RegexpMatcherManager::Instance ().Remove (index);
}

