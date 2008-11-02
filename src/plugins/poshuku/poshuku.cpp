#include "poshuku.h"
#include <QMessageBox>
#include "core.h"

void Poshuku::Init ()
{
	Ui_.setupUi (this);
	Ui_.ActionSettings_->setProperty ("ActionIcon", "poshuku_preferences");
	Ui_.SettingsButton_->setDefaultAction (Ui_.ActionSettings_);

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
}

void Poshuku::Release ()
{
	Core::Instance ().Release ();
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

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, Poshuku);

