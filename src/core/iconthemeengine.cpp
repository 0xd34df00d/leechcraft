/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "iconthemeengine.h"
#include <algorithm>
#include <QAction>
#include <QPushButton>
#include <QTabWidget>
#include <QIcon>
#include <QDir>
#include <QApplication>
#include <QToolButton>
#include <QTimer>
#include <QMenu>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "childactioneventfilter.h"
#include "util/util.h"
#include <util/sys/resourceloader.h>
#include "util/sys/paths.h"

using namespace LC;

IconThemeEngine::IconThemeEngine ()
: PluginIconLoader_ { std::make_shared<Util::ResourceLoader> ("global_icons/plugins")}
{
	PluginIconLoader_->AddGlobalPrefix ();
	PluginIconLoader_->AddLocalPrefix ();

	QTimer *timer = new QTimer (this);
	connect (timer,
			SIGNAL (timeout ()),
			this,
			SLOT (flushCaches ()));
	timer->start (60000);

#ifdef Q_OS_WIN32
	QIcon::setThemeSearchPaths ({ qApp->applicationDirPath () + "/icons/" });
#elif defined (Q_OS_MAC)
	#ifdef USE_UNIX_LAYOUT
	const auto& envPath = qgetenv ("LC_ICONSETS_PATH");
	if (!envPath.isEmpty ())
		QIcon::setThemeSearchPaths ({ QString::fromUtf8 (envPath) });
	#else
	QIcon::setThemeSearchPaths ({ qApp->applicationDirPath () + "/../Resources/icons/" });
	#endif
#endif

	const QDir& dir = Util::CreateIfNotExists ("/icons/");
	QIcon::setThemeSearchPaths (QStringList { dir.absolutePath () } + QIcon::themeSearchPaths ());

	FindIconSets ();
}

IconThemeEngine& IconThemeEngine::Instance ()
{
	static IconThemeEngine e;
	return e;
}

QIcon IconThemeEngine::GetIcon (const QString& actionIcon, const QString& actionIconOff)
{
	const auto& namePair = qMakePair (actionIcon, actionIconOff);

	{
		QReadLocker locker { &IconCacheLock_ };
		if (IconCache_.contains (namePair))
			return IconCache_ [namePair];
	}

	if (QIcon::hasThemeIcon (actionIcon) &&
			(actionIconOff.isEmpty () ||
			 QIcon::hasThemeIcon (actionIconOff)))
	{
		auto result = QIcon::fromTheme (actionIcon);
		if (!actionIconOff.isEmpty ())
		{
			const auto& off = QIcon::fromTheme (actionIconOff);
			for (const auto& size : off.availableSizes ())
				result.addPixmap (off.pixmap (size, QIcon::Normal, QIcon::On));
		}

		{
			QWriteLocker locker { &IconCacheLock_ };
			IconCache_ [namePair] = result;
		}

		return result;
	}

#ifdef QT_DEBUG
	qDebug () << Q_FUNC_INFO << "no icon for" << actionIcon << actionIconOff << QIcon::themeName () << QIcon::themeSearchPaths ();
#endif

	return QIcon ();
}

void IconThemeEngine::UpdateIconset (const QList<QAction*>& actions)
{
	FindIcons ();

	for (auto action : actions)
	{
		if (action->menu ())
			UpdateIconset (action->menu ()->actions ());

		if (action->property ("WatchActionIconChange").toBool ())
			action->installEventFilter (this);

		if (!action->property ("ActionIcon").isValid ())
			continue;

		SetIcon (action);
	}
}

void IconThemeEngine::UpdateIconset (const QList<QPushButton*>& buttons)
{
	FindIcons ();

	for (auto button : buttons)
	{
		if (!button->property ("ActionIcon").isValid ())
			continue;

		SetIcon (button);
	}
}

void IconThemeEngine::UpdateIconset (const QList<QToolButton*>& buttons)
{
	for (auto button : buttons)
	{
		if (!button->property ("ActionIcon").isValid ())
			continue;

		SetIcon (button);
	}
}

void IconThemeEngine::ManageWidget (QWidget *widget)
{
	UpdateIconset (widget->findChildren<QAction*> ());
	UpdateIconset (widget->findChildren<QPushButton*> ());

	widget->installEventFilter (new ChildActionEventFilter (widget));
}

QStringList IconThemeEngine::ListIcons () const
{
	return IconSets_;
}

QIcon IconThemeEngine::GetPluginIcon (const QString& basename)
{
	if (const auto pos = PluginIconCache_.find (basename);
			pos != PluginIconCache_.end ())
		return *pos;

	QPixmap px;

	if (const auto& pluginsIconset = XmlSettingsManager::Instance ()->property ("PluginsIconset").toString ();
			pluginsIconset != "Default")
		px = PluginIconLoader_->LoadPixmap (pluginsIconset + "/" + basename);

	if (px.isNull ())
		px = PluginIconLoader_->LoadPixmap ("default/" + basename);

	if (!px.isNull ())
		PluginIconCache_ [basename] = px;

	return px;
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

template<typename T>
void IconThemeEngine::SetIcon (T iconable)
{
	QString actionIcon = iconable->property ("ActionIcon").toString ();
	QString actionIconOff = iconable->property ("ActionIconOff").toString ();

	iconable->setIcon (GetIcon (actionIcon, actionIconOff));
}

void IconThemeEngine::FindIconSets ()
{
	IconSets_.clear ();

	for (const auto& str : QIcon::themeSearchPaths ())
	{
		QDir dir { str };
		for (const auto& subdirName : dir.entryList (QDir::Dirs))
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
	const auto& iconSet = XmlSettingsManager::Instance ()->property ("IconSet").toString ();

	if (iconSet != OldIconSet_)
	{
		QIcon::setThemeName (iconSet);

		flushCaches ();
		OldIconSet_ = iconSet;

		emit iconsetChanged ();
	}
}

void IconThemeEngine::flushCaches ()
{
	IconCache_.clear ();
}
