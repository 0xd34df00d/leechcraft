#include "poshuku.h"
#include <QMessageBox>
#include <QWebSettings>
#include <QUrl>
#include <plugininterface/util.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "filtermodel.h"

void Poshuku::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));
	Ui_.setupUi (this);
	Ui_.ActionSettings_->setProperty ("ActionIcon", "poshuku_preferences");
	Ui_.SettingsButton_->setDefaultAction (Ui_.ActionSettings_);

	XmlSettingsDialog_.reset (new XmlSettingsDialog (this));
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			":/poshukusettings.xml");

	connect (&Core::Instance (),
			SIGNAL (addNewTab (const QString&, QWidget*)),
			this,
			SIGNAL (addNewTab (const QString&, QWidget*)));
	connect (&Core::Instance (),
			SIGNAL (removeTab (QWidget*)),
			this,
			SIGNAL (removeTab (QWidget*)));
	connect (&Core::Instance (),
			SIGNAL (changeTabName (QWidget*, const QString&)),
			this,
			SIGNAL (changeTabName (QWidget*, const QString&)));
	connect (&Core::Instance (),
			SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
			this,
			SIGNAL (changeTabIcon (QWidget*, const QIcon&)));

	XmlSettingsManager::Instance ()->RegisterObject ("StandardFont",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("FixedFont",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("SerifFont",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("SansSerifFont",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("CursiveFont",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("FantasyFont",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("MinimumFontSize",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("DefaultFontSize",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("DefaultFixedFontSize",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("AutoLoadImages",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("AllowJavaScript",
			this, "viewerSettingsChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("UserStyleSheet",
			this, "viewerSettingsChanged");

	Ui_.FavoritesView_->setModel (Core::Instance ().GetFavoritesModel ());

	FavoritesFilterLineCompleter_.reset (new TagsCompleter (Ui_.FavoritesFilterLine_, this));
	FavoritesFilterLineCompleter_->
		setModel (Core::Instance ().GetFavoritesTagsCompletionModel ());
	Ui_.FavoritesFilterLine_->AddSelector ();
	connect (Ui_.FavoritesFilterLine_,
			SIGNAL (textChanged (const QString&)),
			this,
			SLOT (updateFavoritesFilter ()));
	connect (Ui_.FavoritesFilterType_,
			SIGNAL (currentIndexChanged (int)),
			this,
			SLOT (updateFavoritesFilter ()));
	connect (Ui_.FavoritesFilterCaseSensitivity_,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (updateFavoritesFilter ()));
}

void Poshuku::Release ()
{
	Core::Instance ().Release ();
	XmlSettingsDialog_.reset ();
}

QString Poshuku::GetName () const
{
	return "Poshuku";
}

QString Poshuku::GetInfo () const
{
	return tr ("Simple yet functional web browser");
}

QStringList Poshuku::Provides () const
{
	return QStringList ("webbrowser");
}

QStringList Poshuku::Needs () const
{
	return QStringList ();
}

QStringList Poshuku::Uses () const
{
	return QStringList ();
}

void Poshuku::SetProvider (QObject*, const QString&)
{
}

QIcon Poshuku::GetIcon () const
{
	return QIcon ();
}

QWidget* Poshuku::GetTabContents ()
{
	return this;
}

void Poshuku::openURL (const QString& url)
{
	Core::Instance ().NewURL (url);
}

QWebView* Poshuku::createWindow ()
{
	return Core::Instance ().MakeWebView ();
}

void Poshuku::on_AddressLine__returnPressed ()
{
	QString url = Ui_.AddressLine_->text ();
	if (!Core::Instance ().IsValidURL (url))
	{
		QMessageBox::critical (this, tr ("Error"),
				tr ("The URL you entered could not be opened by Poshuku. "
					"Sorry. By the way, you entered:<br />%1").arg (url));
		return;
	}

	Core::Instance ().NewURL (url);
}

void Poshuku::on_ActionSettings__triggered ()
{
	XmlSettingsDialog_->show ();
}

void Poshuku::viewerSettingsChanged ()
{
	QWebSettings::globalSettings ()->setFontFamily (QWebSettings::StandardFont,
			XmlSettingsManager::Instance ()->property ("StandardFont").value<QFont> ().family ());
	QWebSettings::globalSettings ()->setFontFamily (QWebSettings::FixedFont,
			XmlSettingsManager::Instance ()->property ("FixedFont").value<QFont> ().family ());
	QWebSettings::globalSettings ()->setFontFamily (QWebSettings::SerifFont,
			XmlSettingsManager::Instance ()->property ("SerifFont").value<QFont> ().family ());
	QWebSettings::globalSettings ()->setFontFamily (QWebSettings::SansSerifFont,
			XmlSettingsManager::Instance ()->property ("SansSerifFont").value<QFont> ().family ());
	QWebSettings::globalSettings ()->setFontFamily (QWebSettings::CursiveFont,
			XmlSettingsManager::Instance ()->property ("CursiveFont").value<QFont> ().family ());
	QWebSettings::globalSettings ()->setFontFamily (QWebSettings::FantasyFont,
			XmlSettingsManager::Instance ()->property ("FantasyFont").value<QFont> ().family ());

	QWebSettings::globalSettings ()->setFontSize (QWebSettings::MinimumFontSize,
			XmlSettingsManager::Instance ()->property ("MinimumFontSize").toInt ());
	QWebSettings::globalSettings ()->setFontSize (QWebSettings::DefaultFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFontSize").toInt ());
	QWebSettings::globalSettings ()->setFontSize (QWebSettings::DefaultFixedFontSize,
			XmlSettingsManager::Instance ()->property ("DefaultFixedFontSize").toInt ());
	QWebSettings::globalSettings ()->setAttribute (QWebSettings::AutoLoadImages,
			XmlSettingsManager::Instance ()->property ("AutoLoadImages").toBool ());
	QWebSettings::globalSettings ()->setAttribute (QWebSettings::JavascriptEnabled,
			XmlSettingsManager::Instance ()->property ("AllowJavaScript").toBool ());
	QWebSettings::globalSettings ()->setUserStyleSheetUrl (QUrl (XmlSettingsManager::
				Instance ()->property ("UserStyleSheet").toString ()));
}

void Poshuku::updateFavoritesFilter ()
{
	int section = Ui_.FavoritesFilterType_->currentIndex ();
	QString text = Ui_.FavoritesFilterLine_->text ();

	switch (section)
	{
		case 1:
			Core::Instance ().GetFavoritesModel ()->setTagsMode (false);
			Core::Instance ().GetFavoritesModel ()->setFilterWildcard (text);
			break;
		case 2:
			Core::Instance ().GetFavoritesModel ()->setTagsMode (false);
			Core::Instance ().GetFavoritesModel ()->setFilterRegExp (text);
			break;
		case 3:
			Core::Instance ().GetFavoritesModel ()->setTagsMode (true);
			Core::Instance ().GetFavoritesModel ()->setFilterFixedString (text);
			break;
		default:
			Core::Instance ().GetFavoritesModel ()->setTagsMode (false);
			Core::Instance ().GetFavoritesModel ()->setFilterFixedString (text);
			break;
	}

	Core::Instance ().GetFavoritesModel ()->
		setFilterCaseSensitivity ((Ui_.FavoritesFilterCaseSensitivity_->
					checkState () == Qt::Checked) ? Qt::CaseSensitive :
				Qt::CaseInsensitive);
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, Poshuku);

