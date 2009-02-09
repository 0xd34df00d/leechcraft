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
	FindIconSets ();
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
			icon = IconName2FileName_ [actionIcon];
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
				offIcon = IconName2FileName_ [actionIconOff];
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
				icon = IconName2FileName_ [*name];
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

QStringList SkinEngine::ListIcons () const
{
	return IconSets_;
}

void SkinEngine::FindIconSets ()
{
	IconSets_.clear ();

#if defined (Q_OS_UNIX)
	QDir dir = QDir ("/usr/share/leechcraft/icons");
	IconSets_ << dir.entryList (QStringList ("*.mapping"));
	dir = QDir ("/usr/local/share/leechcraft/icons");
	IconSets_ << dir.entryList (QStringList ("*.mapping"));
#elif defined (Q_OS_WIN32)
	QDir dir = QDir::current ();
	if (dir.cd ("icons"))
		IconSets_ << dir.entryList (QStringList ("*.mapping"));
#endif

	dir = QDir::home ();
	if (dir.cd (".icons"))
		IconSets_ << dir.entryList (QStringList ("*.mapping"));
	dir = QDir::home ();
	if (dir.cd (".leechcraft") && dir.cd ("icons"))
		IconSets_ << dir.entryList (QStringList ("*.mapping"));

	for (QStringList::iterator i = IconSets_.begin (),
			end = IconSets_.end (); i != end; ++i)
		*i = i->left (i->size () - 8);
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

		FillMapping (QDir::homePath () + "/.icons", iconSet);
		FillMapping (QDir::homePath () + "/.leechcraft/icons", iconSet);
		CollectDir (QDir::homePath () + "/.icons", iconSet);
		CollectDir (QDir::homePath () + "/.leechcraft/icons", iconSet);

#if defined (Q_OS_UNIX)
		FillMapping ("/usr/share/leechcraft/icons", iconSet);
		FillMapping ("/usr/local/share/leechcraft/icons", iconSet);
		CollectDir ("/usr/share/icons", iconSet);
		CollectDir ("/usr/local/share/icons", iconSet);
#elif defined (Q_OS_WIN32)
		FillMapping (QApplication::applicationDirPath () + "/icons", iconSet);
		CollectDir (QApplication::applicationDirPath () + "/icons", iconSet);
#endif
	}
}

void SkinEngine::FillMapping (const QString& folder, const QString& iconSet)
{
	QDir dir (folder);

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
		if (*i == 0)
		{
			current.cd ("scalable");

			QStringList subdirs = current.entryList (QStringList (),
					QDir::Dirs | QDir::NoDotAndDotDot);

			for (QStringList::const_iterator j = subdirs.begin (),
					subdirsEnd = subdirs.end (); j != subdirsEnd; ++j)
				CollectSubdir (current, *j, *i);
		}
		else if (*i >= 16 && *i <= 32)
		{
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);

			QStringList subdirs = current.entryList (QStringList (),
					QDir::Dirs | QDir::NoDotAndDotDot);

			for (QStringList::const_iterator j = subdirs.begin (),
					subdirsEnd = subdirs.end (); j != subdirsEnd; ++j)
				CollectSubdir (current, *j, *i);
		}
	}
}

void SkinEngine::CollectSubdir (QDir current, const QString& dir, int size)
{
	current.cd (dir);
	QFileInfoList infos =
		current.entryInfoList (QStringList ("*.png") << "*.svg",
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
		if (*i == "scalable")
		{
			numbers.push_back (0);
			continue;
		}

		QStringList splitted = i->split ('x');
		if (splitted.size () != 2)
			continue;

		numbers.push_back (splitted.at (0).toInt ());
	}

	std::sort (numbers.begin (), numbers.end ());
	return numbers;
}

