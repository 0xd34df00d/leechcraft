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

#ifndef PLUGINS_SEEKTHRU_SEEKTHRU_H
#define PLUGINS_SEEKTHRU_SEEKTHRU_H
#include <QObject>
#include <QTranslator>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SeekThru : public QObject
						   , public IInfo
						   , public IFinder
						   , public IHaveSettings
						   , public IEntityHandler
						   , public IStartupWizard
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IFinder IHaveSettings IEntityHandler IStartupWizard)

				std::auto_ptr<QTranslator> Translator_;
				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> XmlSettingsDialog_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				QStringList GetCategories () const;
				QList<IFindProxy_ptr> GetProxy (const LeechCraft::Request&);

				boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> GetSettingsDialog () const;

				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);

				QList<QWizardPage*> GetWizardPages () const;
			private slots:
				void handleError (const QString&);
				void handleWarning (const QString&);
			signals:
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void categoriesChanged (const QStringList&, const QStringList&);
			};
		};
	};
};

#endif

