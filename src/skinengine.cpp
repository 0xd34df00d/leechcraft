#include "skinengine.h"
#include <algorithm>
#include <QAction>
#include <QTabWidget>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QApplication>
#include <QtDebug>
#include "xmlsettingsmanager.h"

using namespace LeechCraft;

SkinEngine::SkinEngine ()
{
}

SkinEngine& SkinEngine::Instance ()
{
	static SkinEngine e;
	return e;
}

SkinEngine::~SkinEngine ()
{
}

void SkinEngine::UpdateIconSet (const QList<QAction*>& actions)
{
	FindIcons ();

	for (QList<QAction*>::const_iterator i = actions.begin (),
			end = actions.end (); i != end; ++i)
	{
		if (!(*i)->property ("ActionIcon").isValid ())
			continue;
		QString actionIcon = (*i)->property ("ActionIcon").toString ();
		QString actionIconOff = (*i)->property ("ActionIconOff").toString ();
		QString icon;
		if (IconName2FileName_.contains (actionIcon))
			icon = IconName2FileName_ [actionIcon] + ".png";
		else
			icon = QString ("lc_") + actionIcon + ".png";

		QIcon iconEntity;
		sizef_t files = IconName2Path_ [icon];
		for (sizef_t::const_iterator sizePair = files.begin ();
				sizePair != files.end (); ++sizePair)
			iconEntity.addFile (sizePair.value (),
					QSize (sizePair.key (), sizePair.key ()),
					QIcon::Normal,
					QIcon::On);

		if (actionIconOff.size ())
		{
			QString offIcon;
			if (IconName2FileName_.contains (actionIconOff))
				offIcon = IconName2FileName_ [actionIconOff] + ".png";
			else
				offIcon = QString ("lc_") + actionIconOff + ".png";

			sizef_t offFiles = IconName2Path_ [offIcon];
			for (sizef_t::const_iterator sizePair = offFiles.begin ();
					sizePair != offFiles.end (); ++sizePair)
				iconEntity.addFile (sizePair.value (),
						QSize (sizePair.key (), sizePair.key ()),
						QIcon::Normal,
						QIcon::Off);
		}
		
		(*i)->setIcon (iconEntity);
	}
}

void SkinEngine::UpdateIconSet (const QList<QTabWidget*>& tabs)
{
	FindIcons ();

	for (QList<QTabWidget*>::const_iterator i = tabs.begin (),
			end = tabs.end (); i != end; ++i)
	{
		QStringList icons = (*i)->property ("TabIcons").toString ()
			.split (" ", QString::SkipEmptyParts);

		int tab = 0;
		for (QStringList::const_iterator name = icons.begin ();
				name != icons.end (); ++name, ++tab)
		{
			QString icon;
			if (IconName2FileName_.contains (*name))
				icon = IconName2FileName_ [*name] + ".png";
			else
				icon = QString ("lc_") + *name + ".png";

			QIcon iconEntity;
			sizef_t files = IconName2Path_ [icon];
			for (sizef_t::const_iterator sizePair = files.begin ();
					sizePair != files.end (); ++sizePair)
				iconEntity.addFile (sizePair.value (),
						QSize (sizePair.key (), sizePair.key ()),
						QIcon::Normal,
						QIcon::On);

			(*i)->setTabIcon (tab, iconEntity);
		}
	}
}

void SkinEngine::FindIcons ()
{
	QString iconSet = XmlSettingsManager::Instance ()->
		property ("IconSet").toString ();

	if (iconSet != OldIconSet_)
	{
		IconName2Path_.clear ();
		IconName2FileName_.clear ();

		OldIconSet_ = iconSet;

#if defined (Q_OS_UNIX)
		QDir dir ("/usr/share/leechcraft/icons");
		if (!dir.exists ())
			dir = "/usr/local/share/leechcraft/icons";
#elif defined (Q_OS_WIN32)
		QDir dir = QApplication::applicationDirPath ();
		dir.cd ("icons");
#endif
		if (dir.exists (iconSet + ".mapping"))
		{
			QFile mappingFile (dir.filePath (iconSet + ".mapping"));
			if (mappingFile.open (QIODevice::ReadOnly))
			{
				QByteArray lineData = mappingFile.readLine ();
				while (!lineData.isEmpty ())
				{
					QStringList pair = QString::fromUtf8 (lineData)
						.split (' ', QString::SkipEmptyParts);
					if (pair.size () == 2)
						IconName2FileName_ [pair.at (0).simplified ()] = pair.at (1).simplified ();

					lineData = mappingFile.readLine ();
				}
			}
		}

#if defined (Q_OS_UNIX)
		CollectDir ("/usr/share/icons", iconSet);
		CollectDir ("/usr/local/share/icons", iconSet);
#elif defined (Q_OS_WIN32)
		CollectDir (QApplication::applicationDirPath (), iconSet);
#endif
	}
}

void SkinEngine::CollectDir (const QString& folder, const QString& iconSet)
{
	std::vector<int> numbers = GetDirForBase (folder, iconSet);
	QDir baseDir (folder);
	baseDir.cd (iconSet);
	for (std::vector<int>::const_iterator i = numbers.begin (),
			end = numbers.end (); i != end; ++i)
	{
		QDir current = baseDir;
		QString number = QString::number (*i);
		int size = number.toInt ();
		if (size > 32 || size < 16)
			continue;

		current.cd (number + 'x' + number);

		QStringList subdirs = current.entryList (QStringList (),
				QDir::Dirs | QDir::NoDotAndDotDot);

		for (QStringList::const_iterator j = subdirs.begin (),
				subdirsEnd = subdirs.end (); j != subdirsEnd; ++j)
			CollectSubdir (current, *j, size);
	}
}

void SkinEngine::CollectSubdir (QDir current, const QString& dir, int size)
{
	current.cd (dir);
	QFileInfoList infos =
		current.entryInfoList (QStringList ("*.png"),
				QDir::Files | QDir::Readable);

	for (QFileInfoList::const_iterator i = infos.begin (),
			infoEnd = infos.end (); i != infoEnd; ++i)
		IconName2Path_ [i->fileName ()] [size] = i->absoluteFilePath ();
}

std::vector<int> SkinEngine::GetDirForBase (const QString& base,
		const QString& iconSet)
{
	QDir baseDir (base);
	baseDir.cd (iconSet);
	std::vector<int> numbers;
	QStringList entries = baseDir.entryList ();
	for (QStringList::const_iterator i = entries.begin (),
			end = entries.end (); i != end; ++i)
	{
		QStringList splitted = i->split ('x');
		if (splitted.size () != 2)
			continue;

		numbers.push_back (splitted.at (0).toInt ());
	}

	std::sort (numbers.begin (), numbers.end ());
	return numbers;
}

