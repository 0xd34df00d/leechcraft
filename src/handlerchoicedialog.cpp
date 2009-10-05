/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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

bool HandlerChoiceDialog::Add (const IInfo *ii, IDownload *id)
{
	QString name;
	QString tooltip;
	try
	{
		name = ii->GetName ();
		tooltip = ii->GetInfo ();
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< "could not query"
			<< e.what ()
			<< ii;
		return false;
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< "could not query"
			<< ii;
		return false;
	}
	QRadioButton *but = new QRadioButton (name, this);
	but->setToolTip (tooltip);
	but->setProperty ("AddedAs", "IDownload");

	if (!Buttons_->buttons ().size ())
		but->setChecked (true);

	Buttons_->addButton (but);
	Ui_.DownloadersLayout_->addWidget (but);
	Downloaders_ [name] = id;

	Ui_.DownloadersLabel_->show ();
	return true;
}

bool HandlerChoiceDialog::Add (const IInfo *ii, IEntityHandler *ih)
{
	QString name;
	QString tooltip;
	try
	{
		name = ii->GetName ();
		tooltip = ii->GetInfo ();
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< "could not query"
			<< e.what ()
			<< ii;
		return false;
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< "could not query"
			<< ii;
		return false;
	}
	QRadioButton *but = new QRadioButton (name, this);
	but->setToolTip (tooltip);
	but->setProperty ("AddedAs", "IEntityHandler");

	if (!Buttons_->buttons ().size ())
		but->setChecked (true);

	Buttons_->addButton (but);
	Handlers_ [name] = ih;
	Ui_.HandlersLayout_->addWidget (but);

	Ui_.HandlersLabel_->show ();
	return true;
}

IDownload* HandlerChoiceDialog::GetDownload ()
{
	IDownload *result = 0;
	if (!Buttons_->checkedButton () ||
			Buttons_->checkedButton ()->
				property ("AddedAs").toString () != "IDownload")
		return 0;
	downloaders_t::iterator rit = Downloaders_.find (Buttons_->
			checkedButton ()->text ());
	if (rit != Downloaders_.end ())
		result = rit->second;
	return result;
}

IDownload* HandlerChoiceDialog::GetFirstDownload ()
{
	if (Downloaders_.size ())
		return Downloaders_.begin ()->second;
	else
		return 0;
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

