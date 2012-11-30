/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "iconthemeengine.h"
#include <algorithm>
#include <QAction>
#include <QTabWidget>
#include <QIcon>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QApplication>
#include <QTimer>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "util/util.h"

using namespace LeechCraft;

const int MaxIconSize = 32;

IconThemeEngine::IconThemeEngine ()
{
	QTimer *timer = new QTimer (this);
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (flushCaches ()));
	timer->start (60000);

#ifdef Q_OS_WIN32
	QIcon::setThemeSearchPaths (QStringList (qApp->applicationDirPath () + "/icons/"));
#elif defined (Q_OS_MAC)
	QIcon::setThemeSearchPaths (QStringList (qApp->applicationDirPath () + "/../Resources/icons/"));
#endif

	const QDir& dir = Util::CreateIfNotExists ("/icons/");
	QIcon::setThemeSearchPaths (QStringList (dir.absolutePath ()) + QIcon::themeSearchPaths ());

	FindIconSets ();
}

IconThemeEngine& IconThemeEngine::Instance ()
{
	static IconThemeEngine e;
	return e;
}

QIcon IconThemeEngine::GetIcon (const QString& actionIcon, const QString& actionIconOff) const
{
	const QPair<QString, QString>& namePair = qMakePair (actionIcon, actionIconOff);
	if (IconCache_.contains (namePair))
		return IconCache_ [namePair];

	if (QIcon::hasThemeIcon (actionIcon) &&
			(actionIconOff.isEmpty () ||
			 QIcon::hasThemeIcon (actionIconOff)))
	{
		QIcon result = QIcon::fromTheme (actionIcon);
		if (!actionIconOff.isEmpty ())
		{
			const QIcon& off = QIcon::fromTheme (actionIconOff);
			Q_FOREACH (const QSize& size, off.availableSizes ())
				result.addPixmap (off.pixmap (size, QIcon::Normal, QIcon::On));
		}
		IconCache_ [namePair] = result;
		return result;
	}

#ifdef QT_DEBUG
	qDebug () << Q_FUNC_INFO << "no icon for" << actionIcon << actionIconOff << QIcon::themeName () << QIcon::themeSearchPaths ();
#endif

	return QIcon ();
}

void IconThemeEngine::UpdateIconSet (const QList<QAction*>& actions)
{
	FindIcons ();

	for (auto action : actions)
	{
		if (action->menu ())
			UpdateIconSet (action->menu ()->actions ());

		if (action->property ("WatchActionIconChange").toBool ())
			action->installEventFilter (this);

		if (!action->property ("ActionIcon").isValid ())
			continue;

		SetIcon (action);
	}
}

void IconThemeEngine::UpdateIconSet (const QList<QTabWidget*>& tabs)
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
			(*i)->setTabIcon (tab, GetIcon (*name, QString ()));
	}
}

QStringList IconThemeEngine::ListIcons () const
{
	return IconSets_;
}

bool IconThemeEngine::eventFilter (QObject *obj, QEvent *e)
{
	if (e->type () != QEvent::DynamicPropertyChange)
		return QObject::eventFilter (obj, e);

	QAction *act = qobject_cast<QAction*> (obj);
	if (!act)
		return QObject::eventFilter (obj, e);

	SetIcon (act);

	return QObject::eventFilter (obj, e);
}

void IconThemeEngine::SetIcon (QAction *act)
{
	QString actionIcon = act->property ("ActionIcon").toString ();
	QString actionIconOff = act->property ("ActionIconOff").toString ();

	act->setIcon (GetIcon (actionIcon, actionIconOff));
}

void IconThemeEngine::FindIconSets ()
{
	IconSets_.clear ();

	Q_FOREACH (const QString& str, QIcon::themeSearchPaths ())
	{
		QDir dir (str);
		Q_FOREACH (const QString& subdirName, dir.entryList (QDir::Dirs))
		{
			QDir subdir (dir);
			subdir.cd (subdirName);
			if (subdir.exists ("index.theme"))
				IconSets_ << subdirName;
		}
	}
}

void IconThemeEngine::FindIcons ()
{
	QString iconSet = XmlSettingsManager::Instance ()->
		property ("IconSet").toString ();

	if (iconSet != OldIconSet_)
	{
		QIcon::setThemeName (iconSet);

		flushCaches ();
		OldIconSet_ = iconSet;
	}
}

void IconThemeEngine::flushCaches ()
{
	IconCache_.clear ();
}
