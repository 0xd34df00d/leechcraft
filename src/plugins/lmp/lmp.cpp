#include "lmp.h"
#include "core.h"
#include "tabwidget.h"

void LMP::Init ()
{
}

void LMP::Release ()
{
	Core::Instance ().Release ();
}

QString LMP::GetName () const
{
	return "LMP";
}

QString LMP::GetInfo () const
{
	return "LeechCraft Media Player";
}

QStringList LMP::Provides () const
{
	return QStringList ("media");
}

QStringList LMP::Needs () const
{
	return QStringList ();
}

QStringList LMP::Uses () const
{
	return QStringList ();
}

void LMP::SetProvider (QObject*, const QString&)
{
}

QIcon LMP::GetIcon () const
{
	return QIcon ();
}

QWidget* LMP::GetTabContents ()
{
	return new TabWidget;
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LMP);

