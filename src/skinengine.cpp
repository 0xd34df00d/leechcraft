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

QMap<int, QString> SkinEngine::GetIconPath (const QString& actionIcon) const
{
	return IconName2Path_ [GetIconName (actionIcon)];
}

QIcon SkinEngine::GetIcon (const QString& actionIcon, const QString& actionIconOff) const
{
	QString icon = GetIconName (actionIcon);

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
		QString offIcon = GetIconName (actionIconOff);

		sizef_t offFiles = IconName2Path_ [offIcon];
		for (sizef_t::const_iterator sizePair = offFiles.begin ();
				sizePair != offFiles.end (); ++sizePair)
			iconEntity.addFile (sizePair.value (),
					QSize (sizePair.key (), sizePair.key ()),
					QIcon::Normal,
					QIcon::Off);
	}

	return iconEntity;
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
		
		(*i)->setIcon (GetIcon (actionIcon, actionIconOff));
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
			QString icon = GetIconName (*name);;

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

QString SkinEngine::GetIconName (const QString& actionIcon) const
{
	QString icon;
	if (IconName2FileName_.contains (actionIcon))
		icon = IconName2FileName_ [actionIcon];
	else
		icon = QString ("lc_") + actionIcon;
	return icon;
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
	if (dir.cd (QCoreApplication::applicationDirPath () + "/icons"))
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

#ifndef Q_NO_DEBUG
	qDebug () << Q_FUNC_INFO
		<< "found"
		<< IconSets_;
#endif
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
			ParseMapping (mappingFile);
		else
			qWarning () << Q_FUNC_INFO
				<< "failed to open mapping file"
				<< iconSet
				<< folder
				<< mappingFile.errorString ();
	}

	if (QFileInfo (dir.filePath (iconSet + ".mapping.d")).isDir ())
	{
		dir.cd (iconSet + ".mapping.d");
		Q_FOREACH (QString entry, dir.entryList (QDir::Files))
		{
			QFile mappingFile (dir.filePath (entry));
			if (mappingFile.open (QIODevice::ReadOnly))
				ParseMapping (mappingFile);
			else
				qWarning () << Q_FUNC_INFO
					<< "failed to open mapping file"
					<< folder
					<< iconSet
					<< "for mapping.d:"
					<< dir.filePath (entry)
					<< mappingFile.errorString ();
		}
	}
}

void SkinEngine::ParseMapping (QFile& mappingFile)
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

	QStringList values = IconName2FileName_.values ();
	for (QFileInfoList::const_iterator i = infos.begin (),
			infoEnd = infos.end (); i != infoEnd; ++i)
		if (values.contains (i->baseName ()) ||
				i->baseName ().startsWith ("lc_"))
			IconName2Path_ [i->baseName ()] [size] = i->absoluteFilePath ();
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

