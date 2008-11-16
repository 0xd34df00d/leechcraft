#include "poshuku.h"
#include <QMessageBox>
#include <QWebSettings>
#include <QHeaderView>
#include <QUrl>
#include <plugininterface/util.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "customwebview.h"
#include "favoritesdelegate.h"

void Poshuku::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("poshuku"));
	Ui_.setupUi (this);

	RegisterSettings ();

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
	connect (&Core::Instance (),
			SIGNAL (error (const QString&)),
			this,
			SLOT (handleError (const QString&)));

	Ui_.HistoryView_->setModel (Core::Instance ().GetHistoryModel ());

	FavoritesFilterModel_.reset (new FilterModel (this));
	FavoritesFilterModel_->setSourceModel (Core::Instance ().GetFavoritesModel ());
	FavoritesFilterModel_->setDynamicSortFilter (true);
	Ui_.FavoritesView_->setModel (FavoritesFilterModel_.get ());
	Ui_.FavoritesView_->setItemDelegate (new FavoritesDelegate (this));
	connect (Ui_.FavoritesView_,
			SIGNAL (deleteSelected (const QModelIndex&)),
			Core::Instance ().GetFavoritesModel (),
			SLOT (removeItem (const QModelIndex&)));

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

	QHeaderView *itemsHeader = Ui_.FavoritesView_->header ();
	QFontMetrics fm = fontMetrics ();
	itemsHeader->resizeSection (0,
			fm.width ("Average site title can be very big, it's also the most important part, so it's priority is the biggest."));
	itemsHeader->resizeSection (1,
			fm.width ("Average URL could be very very long, but we don't account this."));
	itemsHeader->resizeSection (2,
			fm.width ("Average tags list size should be like this."));
}

void Poshuku::Release ()
{
	Core::Instance ().Release ();
	XmlSettingsDialog_.reset ();
	FavoritesFilterLineCompleter_.reset ();
	FavoritesFilterModel_.reset ();
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

void Poshuku::RegisterSettings ()
{
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
			FavoritesFilterModel_->setTagsMode (false);
			FavoritesFilterModel_->setFilterWildcard (text);
			break;
		case 2:
			FavoritesFilterModel_->setTagsMode (false);
			FavoritesFilterModel_->setFilterRegExp (text);
			break;
		case 3:
			FavoritesFilterModel_->setTagsMode (true);
			FavoritesFilterModel_->setFilterFixedString (text);
			break;
		default:
			FavoritesFilterModel_->setTagsMode (false);
			FavoritesFilterModel_->setFilterFixedString (text);
			break;
	}

	FavoritesFilterModel_->
		setFilterCaseSensitivity ((Ui_.FavoritesFilterCaseSensitivity_->
					checkState () == Qt::Checked) ? Qt::CaseSensitive :
				Qt::CaseInsensitive);
}

void Poshuku::handleError (const QString& msg)
{
	QMessageBox::warning (this, tr ("Error"), msg);
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, Poshuku);

