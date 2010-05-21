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

#ifndef CLIPBOARDWATCHER_H
#define CLIPBOARDWATCHER_H
#include <QObject>
#include <QString>
#include <interfaces/structures.h>

namespace LeechCraft
{
	/** @brief Watches clipboard for downloadable contents.
	 *
	 * ClipboardWatcher polls the clipboard periodically for new content
	 * and emits gotEntity() if it's found.
	 *
	 * The entities emitted from ClipboardWatcher do have
	 * LeechCraft::FromUserInitiated flag set, Entity_ is an utf8-ed
	 * text from the clipboard.
	 */
	class ClipboardWatcher : public QObject
	{
		Q_OBJECT

		QTimer *ClipboardWatchdog_;
		QString PreviousClipboardContents_;
	public:
		ClipboardWatcher (QObject *parent = 0);
		/** Stops the polling timer and destructs the watcher.
		 */
		virtual ~ClipboardWatcher ();
	private slots:
		/** Checks the clipboard for new content and whether it could
		 * be handled.
		 */
		void handleClipboardTimer ();
	signals:
		/** Notifies about new entity obtained from the clipboard.
		 */
		void gotEntity (const LeechCraft::Entity&);
	};
};

#endif

