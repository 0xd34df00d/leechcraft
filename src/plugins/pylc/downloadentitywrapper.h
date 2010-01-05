/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_PYLC_DOWNLOADENTITYWRAPPER_H
#define PLUGINS_PYLC_DOWNLOADENTITYWRAPPER_H
#include <QObject>
#include <QVariantMap>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			class DownloadEntityWrapper : public QObject
			{
				Q_OBJECT
				Q_ENUMS (TaskParameters)
				
				DownloadEntity W_;
			public:
				DownloadEntityWrapper (const DownloadEntity&);
				Q_PROPERTY (QVariant Entity READ GetEntity WRITE SetEntity)
				Q_PROPERTY (QString Location READ GetLocation WRITE SetLocation)
				Q_PROPERTY (QString Mime READ GetMime WRITE SetMime)
				Q_PROPERTY (TaskParameters Parameters READ GetParameters WRITE SetParameters)
				Q_PROPERTY (QVariantMap Additional READ GetAdditional WRITE SetAdditional)

				enum TaskParameter
				{
					/** Use default parameters.
					 */
					NoParameters = 0,
					/** Task should not be started automatically after addition.
					 */
					NoAutostart = 1,
					/** Task should not be saved in history.
					 */
					DoNotSaveInHistory = 2,
					/** Task is really downloaded, so, a file, for example, has
					 * appeared as a result.
					 */
					IsDownloaded = 4,
					/** Task is created as a result of user's actions.
					 */
					FromUserInitiated = 8,
					/** User should not be notified about task finish.
					 */
					DoNotNotifyUser = 32,
					/** Task is used internally and would not be visible to the user
					 * at all.
					 */
					Internal = 64,
					/** Task should not be saved as it would have no meaning after
					 * next start.
					 */
					NotPersistent = 128,
					/** When the task is finished, it should not be announced via
					 * gotEntity() signal.
					 */
					DoNotAnnounceEntity = 256,
					/** This task should not be downloaded, only handled by a
					 * handler.
					 */
					OnlyHandle = 512,
					/** This task should not be handled, only downloaded by a
					 * downloader.
					 */
					OnlyDownload = 1024,
					/** This task should be automatically accepted if any handler is
					 * available.
					 */
					AutoAccept = 2048,
					/** The plugin that was the source of this task should be
					 * queried if it could handle the task.
					 */
					ShouldQuerySource = 4096
				};
			public slots:
				QVariant GetEntity () const;
				QString GetLocation () const;
				QString GetMime () const;
				TaskParameters GetParameters () const;
				QMap<QString, QVariant> GetAdditional () const;
				void SetEntity (const QVariant&);
				void SetLocation (const QString&);
				void SetMime (const QString&);
				void SetParameters (const TaskParameters&);
				void SetAdditional (const QMap<QString, QVariant>&);
			}; 
		};
	};
};

#endif

