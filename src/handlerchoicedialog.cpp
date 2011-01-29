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

		if (Buttons_->buttons ().isEmpty ())
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

		if (Buttons_->buttons ().isEmpty ())
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
		if (!Buttons_->checkedButton () ||
				Buttons_->checkedButton ()->
					property ("AddedAs").toString () != "IDownload")
			return 0;

		const downloaders_t::const_iterator& rit = Downloaders_.find (Buttons_->
				checkedButton ()->text ());
		return rit != Downloaders_.end () ? rit->second : 0;
	}

	IDownload* HandlerChoiceDialog::GetFirstDownload ()
	{
		return Downloaders_.empty () ? 0 : Downloaders_.begin ()->second;
	}

	IEntityHandler* HandlerChoiceDialog::GetEntityHandler ()
	{
		const handlers_t::const_iterator& rit = Handlers_.find (Buttons_->
				checkedButton ()->text ());
		return rit != Handlers_.end () ? rit->second : 0;
	}

	IEntityHandler* HandlerChoiceDialog::GetFirstEntityHandler ()
	{
		return Handlers_.empty () ? 0 : Handlers_.begin ()->second;
	}

	QString HandlerChoiceDialog::GetFilename () const
	{
		const QString& name = Buttons_->checkedButton ()->
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
		
		const QStringList& l = settings.value (plugin).toStringList ();
		settings.endGroup ();
		return l;
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

		const QString& plugin = checked->property ("PluginName").toString ();
		const QStringList& pluginTexts = GetPluginSavePaths (plugin).mid (0, 7);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
		settings.beginGroup ("SavePaths");
		QStringList otherPlugins = settings.childKeys ();
		settings.endGroup ();

		otherPlugins.removeAll (plugin);
		QList<QStringList> otherTextsList;
		Q_FOREACH (const QString& otherPlugin, otherPlugins)
			otherTextsList.append (GetPluginSavePaths (otherPlugin));

		for (QList<QStringList>::iterator it = otherTextsList.begin (), end = otherTextsList.end ();
			 it != end; ++it)
			Q_FOREACH (const QString& ptext, pluginTexts)
				it->removeAll (ptext);

		QStringList otherTexts;
		while (otherTexts.size () < 16)
		{
			bool added = false;
			for (QList<QStringList>::iterator it = otherTextsList.begin (), end = otherTextsList.end ();
				 it != end; ++it)
			{
				if (!it->isEmpty ())
				{
					otherTexts += it->takeFirst ();
					added = true;
				}
			}
			if (!added)
				break;
		}

		if (!pluginTexts.isEmpty ())
		{
			Ui_.LocationsBox_->addItems (pluginTexts);
			if (!otherTexts.isEmpty ())
				Ui_.LocationsBox_->insertSeparator (pluginTexts.size () + 2);
		}
		Ui_.LocationsBox_->addItems (otherTexts);

		if (!Suggestion_.isEmpty ())
			Ui_.LocationsBox_->setCurrentIndex (1);
		else
		{
			const QString& prev = settings.value ("PreviousEntitySavePath").toString ();
			if (!prev.isEmpty () &&
					pluginTexts.contains (prev))
			{
				const int pos = Ui_.LocationsBox_->findText (prev);
				if (pos != -1)
					Ui_.LocationsBox_->setCurrentIndex (pos);
			}
			else if (!pluginTexts.isEmpty ())
				Ui_.LocationsBox_->setCurrentIndex (2);
		}
	}
};

