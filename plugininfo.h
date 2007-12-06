#ifndef PLUGININFO_H
#define PLUGININFO_H
#include <QString>
#include <QStringList>
#include <QIcon>

class PluginInfo
{
	QString Name_, Info_;
	QIcon Icon_;
	QString StatusBarMessage_;
	QStringList Provides_, Needs_, Uses_;
public:
	PluginInfo (const QString&, const QString&, const QIcon&, const QString&, const QStringList&, const QStringList&, const QStringList&);
	void SetTooltipMessage (const QString&);
	const QString& GetName () const;
	const QString& GetInfo () const;
	const QIcon& GetIcon () const;
	const QString& GetStatusbarMessage () const;
	const QStringList& GetProvides () const;
	const QStringList& GetNeeds () const;
	const QStringList& GetUses () const;
};

#endif

