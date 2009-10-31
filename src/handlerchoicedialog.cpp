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
#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	HandlerChoiceDialog::HandlerChoiceDialog (const QString& entity, QWidget *parent)
	: QDialog (parent)
	, Buttons_ (new QButtonGroup)
	{
		Ui_.setupUi (this);
		Ui_.EntityLabel_->setText (entity);
		Ui_.DownloadersLabel_->hide ();
		Ui_.HandlersLabel_->hide ();

		connect (Buttons_.get (),
				SIGNAL (buttonReleased (int)),
				this,
				SLOT (populateLocationsBox ()));
	}

	void HandlerChoiceDialog::SetFilenameSuggestion (const QString& location)
	{
		Suggestion_ = location;
	}

	bool HandlerChoiceDialog::Add (const IInfo *ii, IDownload *id)
	{
		QString name;
		QString tooltip;
		QIcon icon;
		try
		{
			name = ii->GetName ();
			tooltip = ii->GetInfo ();
			icon = ii->GetIcon ();
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
		but->setIconSize (QSize (32, 32));
		but->setIcon (icon);
		but->setProperty ("AddedAs", "IDownload");
		but->setProperty ("PluginName", name);

		if (!Buttons_->buttons ().size ())
			but->setChecked (true);

		Buttons_->addButton (but);
		Ui_.DownloadersLayout_->addWidget (but);
		Downloaders_ [name] = id;

		Ui_.DownloadersLabel_->show ();

		if (Downloaders_.size () + Handlers_.size () == 1)
			populateLocationsBox ();

		return true;
	}

	bool HandlerChoiceDialog::Add (const IInfo *ii, IEntityHandler *ih)
	{
		QString name;
		QString tooltip;
		QIcon icon;
		try
		{
			name = ii->GetName ();
			tooltip = ii->GetInfo ();
			icon = ii->GetIcon ();
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
		but->setIconSize (QSize (32, 32));
		but->setIcon (icon);
		but->setProperty ("AddedAs", "IEntityHandler");
		but->setProperty ("PluginName", name);

		if (!Buttons_->buttons ().size ())
			but->setChecked (true);

		Buttons_->addButton (but);
		Handlers_ [name] = ih;
		Ui_.HandlersLayout_->addWidget (but);

		Ui_.HandlersLabel_->show ();

		if (Downloaders_.size () + Handlers_.size () == 1)
			populateLocationsBox ();

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

	QString HandlerChoiceDialog::GetFilename () const
	{
		QString name = Buttons_->checkedButton ()->
			property ("PluginName").toString ();

		QString result;
		if (Ui_.LocationsBox_->currentIndex () == 0)
		{
			if (Suggestion_.isEmpty ())
				Suggestion_ = GetPluginSavePaths (name).value (0, QDir::homePath ());

			result = QFileDialog::getExistingDirectory (0,
					tr ("Select save location"),
					Suggestion_,
					QFileDialog::Option (~QFileDialog::ShowDirsOnly));
			if (result.isEmpty ())
				return QString ();
		}
		else
			result = Ui_.LocationsBox_->currentText ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
		settings.setValue ("PreviousEntitySavePath", result);

		settings.beginGroup ("SavePaths");
		QStringList pluginTexts = settings.value (name).toStringList ();
		pluginTexts.removeAll (result);
		pluginTexts.prepend (result);
		pluginTexts = pluginTexts.mid (0, 20);
		settings.setValue (name, pluginTexts);
		settings.endGroup ();

		return result;
	}

	int HandlerChoiceDialog::NumChoices () const
	{
		return Buttons_->buttons ().size ();
	}

	QStringList HandlerChoiceDialog::GetPluginSavePaths (const QString& plugin) const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
		settings.beginGroup ("SavePaths");
		QStringList pluginTexts = settings.value (plugin).toStringList ();
		settings.endGroup ();
		return pluginTexts;
	}

	void HandlerChoiceDialog::populateLocationsBox ()
	{
		while (Ui_.LocationsBox_->count () > 1)
			Ui_.LocationsBox_->removeItem (1);

		QAbstractButton *checked = Buttons_->checkedButton ();
		if (!checked)
			return;

		if (checked->property ("AddedAs").toString () == "IEntityHandler")
		{
			Ui_.LocationsBox_->setEnabled (false);
			return;
		}
		Ui_.LocationsBox_->setEnabled (true);

		Ui_.LocationsBox_->insertSeparator (1);

		if (Suggestion_.size ())
			Ui_.LocationsBox_->addItem (Suggestion_);

		QString plugin = checked->property ("PluginName").toString ();
		QStringList pluginTexts = GetPluginSavePaths (plugin).mid (0, 7);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
		settings.beginGroup ("SavePaths");
		QStringList otherPlugins = settings.childKeys ();
		settings.endGroup ();
		QStringList otherTexts;
		otherPlugins.removeAll (plugin);
		QList<QStringList> otherTextsList;
		Q_FOREACH (QString otherPlugin, otherPlugins)
			otherTextsList.append (GetPluginSavePaths (otherPlugin));
		while (otherTexts.size () < 16)
		{
			bool added = false;
			for (int i = 0; i < otherTextsList.size (); ++i)
			{
				if (otherTextsList.at (i).size ())
				{
					otherTexts += otherTextsList [i].takeFirst ();
					added = true;
				}
			}
			if (!added)
				break;
		}

		if (pluginTexts.size ())
		{
			Ui_.LocationsBox_->addItems (pluginTexts);
			if (otherTexts.size ())
				Ui_.LocationsBox_->insertSeparator (pluginTexts.size () + 2);
		}
		Ui_.LocationsBox_->addItems (otherTexts);

		if (Suggestion_.size ())
			Ui_.LocationsBox_->setCurrentIndex (1);
		else
		{
			QString prev = settings.value ("PreviousEntitySavePath").toString ();
			if (prev.size () &&
					pluginTexts.contains (prev))
			{
				int pos = Ui_.LocationsBox_->findText (prev);
				if (pos != -1)
					Ui_.LocationsBox_->setCurrentIndex (pos);
			}
			else if (pluginTexts.size ())
				Ui_.LocationsBox_->setCurrentIndex (2);
		}
	}
};

