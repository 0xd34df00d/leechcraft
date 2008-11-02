#include "poshuku.h"

void Poshuku::Init ()
{
	Ui_.setupUi (this);
}

void Poshuku::Release ()
{
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

Q_EXPORT_PLUGIN2 (leechcraft_poshuku, Poshuku);

