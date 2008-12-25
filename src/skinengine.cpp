#include "skinengine.h"
#include <algorithm>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
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
					if (pair.size () != 2)
						continue;

					IconName2FileName_ [pair.at (0).simplified ()] = pair.at (1).simplified ();

					lineData = mappingFile.readLine ();
				}
			}
		}

#if defined (Q_OS_UNIX)
		std::vector<int> numbers = GetDirForBase ("/usr/share/icons", iconSet);
		QDir baseDir ("/usr/share/icons");
		baseDir.cd (iconSet);
		for (std::vector<int>::const_iterator i = numbers.begin (),
				end = numbers.end (); i != end; ++i)
		{
			QDir current = baseDir;
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);
			current.cd ("actions");
			QFileInfoList infos =
				current.entryInfoList (QStringList ("lc_*.png"),
						QDir::Files | QDir::Readable);

			for (QFileInfoList::const_iterator j = infos.begin (),
					infoEnd = infos.end (); j != infoEnd; ++j)
				IconName2Path_ [j->fileName ()] = j->absoluteFilePath ();
		}

		baseDir = QDir ("/usr/local/share/icons");
		numbers = GetDirForBase ("/usr/local/share/icons", iconSet);
		baseDir.cd (iconSet);
		for (std::vector<int>::const_iterator i = numbers.begin (),
				end = numbers.end (); i != end; ++i)
		{
			QDir current = baseDir;
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);
			current.cd ("actions");
			QFileInfoList infos =
				current.entryInfoList (QStringList ("lc_*.png"),
						QDir::Files | QDir::Readable);

			for (QFileInfoList::const_iterator j = infos.begin (),
					infoEnd = infos.end (); j != infoEnd; ++j)
				IconName2Path_ [j->fileName ()] = j->absoluteFilePath ();
		}
#elif defined (Q_OS_WIN32)
		QDir baseDir = QApplication::applicationDirPath ();
		baseDir.cd ("icons");
		std::vector<int> numbers = GetDirForBase (baseDir.absolutePath (), iconSet);
		baseDir.cd (iconSet);
		for (std::vector<int>::const_iterator i = numbers.begin (),
				end = numbers.end (); i != end; ++i)
		{
			QDir current = baseDir;
			QString number = QString::number (*i);
			current.cd (number + 'x' + number);
			current.cd ("actions");
			QFileInfoList infos =
				current.entryInfoList (QStringList ("lc_*.png"),
						QDir::Files | QDir::Readable);

			for (QFileInfoList::const_iterator j = infos.begin (),
					infoEnd = infos.end (); j != infoEnd; ++j)
				IconName2Path_ [j->fileName ()] = j->absoluteFilePath ();
		}
#endif
	}
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

void SkinEngine::updateIconSet (const QList<QAction*>& actions)
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
		iconEntity.addPixmap (QPixmap (IconName2Path_ [icon]),
				QIcon::Normal,
				QIcon::On);

		if (actionIconOff.size ())
		{
			QString offIcon;
			if (IconName2FileName_.contains (actionIconOff))
				offIcon = IconName2FileName_ [actionIconOff] + ".png";
			else
				offIcon = QString ("lc_") + actionIconOff + ".png";
			iconEntity.addPixmap (IconName2Path_ [offIcon],
					QIcon::Normal,
					QIcon::Off);
		}
		
		(*i)->setIcon (iconEntity);
	}
}

