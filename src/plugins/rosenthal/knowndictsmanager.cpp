/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "knowndictsmanager.h"
#include <QFileSystemWatcher>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QCoreApplication>
#include <util/util.h>
#include <util/sys/paths.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Rosenthal
{
	namespace
	{
		QStringList GetSystemPaths ()
		{
			static const QStringList candidates
			{
#ifdef Q_OS_WIN32
				QCoreApplication::applicationDirPath () + "/myspell/"
#else
				"/usr/local/share/myspell/",
				"/usr/share/myspell/",
				"/usr/local/share/myspell/dicts/",
				"/usr/share/myspell/dicts/",
				"/usr/local/share/hunspell/",
				"/usr/share/hunspell/"
#endif
			};

			return candidates;
		}
	}

	KnownDictsManager::KnownDictsManager ()
	: LocalPath_ (Util::CreateIfNotExists ("data/dicts/myspell").absolutePath () + '/')
	, Model_ (new QStandardItemModel (this))
	, EnabledModel_ (new QStringListModel (this))
	{
		auto watcher = new QFileSystemWatcher (this);
		watcher->addPath (LocalPath_);
		for (const auto& path : GetSystemPaths ())
			if (QFile::exists (path))
				watcher->addPath (path);

		LoadSettings ();

		connect (watcher,
				SIGNAL (directoryChanged (QString)),
				this,
				SLOT (rebuildDictsModel ()));
		rebuildDictsModel ();

		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));
		XmlSettingsManager::Instance ().RegisterObject ("PrimaryLanguage",
				this, "reemitLanguages");
	}

	QAbstractItemModel* KnownDictsManager::GetModel () const
	{
		return Model_;
	}

	QStringList KnownDictsManager::GetLanguages () const
	{
		auto langs = Languages_;
		const auto& primary = XmlSettingsManager::Instance ()
				.property ("PrimaryLanguage").toString ();
		if (langs.removeOne (primary))
			langs.prepend (primary);
		return langs;
	}

	QString KnownDictsManager::GetDictPath (const QString& language) const
	{
		return Lang2Path_ [language] + language;
	}

	QAbstractItemModel* KnownDictsManager::GetEnabledModel () const
	{
		return EnabledModel_;
	}

	void KnownDictsManager::LoadSettings ()
	{
		const QStringList defLangs { Util::GetLocaleName (), "en_GB" };
		Languages_ = XmlSettingsManager::Instance ()
				.Property ("EnabledLanguages", defLangs).toStringList ();
		EnabledModel_->setStringList (Languages_);
	}

	void KnownDictsManager::SaveSettings ()
	{
		XmlSettingsManager::Instance ().setProperty ("EnabledLanguages", Languages_);
	}

	void KnownDictsManager::rebuildDictsModel ()
	{
		auto candidates = GetSystemPaths ();
		candidates.prepend (LocalPath_);

		Lang2Path_.clear ();
		for (const auto& dir : candidates)
		{
			if (!QFile::exists (dir))
				continue;

			for (auto file : QDir (dir).entryList ({ "*.dic" }))
			{
				if (file.startsWith ("hyph_"))
					continue;

				file.chop (4);
				if (Lang2Path_.contains (file))
					continue;

				Lang2Path_ [file] = dir;
			}
		}

		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Locale"), tr ("Language"), tr ("Country") });

		for (auto i = Lang2Path_.begin (); i != Lang2Path_.end (); ++i)
		{
			auto item = new QStandardItem (i.key ());
			item->setCheckable (true);
			item->setCheckState (Languages_.contains (i.key ()) ? Qt::Checked : Qt::Unchecked);

			const QLocale loc (i.key ());

			QList<QStandardItem*> row { item };
			row << new QStandardItem (loc.nativeLanguageName ());
			row << new QStandardItem (loc.nativeCountryName ());

			for (auto item : row)
				item->setEditable (false);

			Model_->appendRow (row);
		}
	}

	void KnownDictsManager::handleItemChanged (QStandardItem *item)
	{
		if (item->column ())
			return;

		const auto& lang = item->text ();
		const auto enabledNow = item->checkState () == Qt::Checked;
		const auto enabled = Languages_.contains (lang);

		if (enabled == enabledNow)
			return;

		if (enabledNow)
			Languages_ << lang;
		else
			Languages_.removeAll (lang);

		EnabledModel_->setStringList (Languages_);

		reemitLanguages() ;

		SaveSettings ();
	}

	void KnownDictsManager::reemitLanguages ()
	{
		emit languagesChanged (GetLanguages ());
	}
}
}
