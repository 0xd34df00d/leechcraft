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

#ifndef SKINENGINE_H
#define SKINENGINE_H
#include <QObject>
#include <QMap>
#include <QString>
#include <QDir>
#include <QHash>

class QIcon;
class QAction;
class QTabWidget;
class QFile;

namespace LeechCraft
{
	class SkinEngine : public QObject
	{
		Q_OBJECT

		QString OldIconSet_;
		QStringList IconSets_;

		mutable QHash<QPair<QString, QString>, QIcon> IconCache_;

		SkinEngine ();
	public:
		static SkinEngine& Instance ();
		virtual ~SkinEngine ();

		QIcon GetIcon (const QString&, const QString&) const;
		void UpdateIconSet (const QList<QAction*>&);
		void UpdateIconSet (const QList<QTabWidget*>&);
		QStringList ListIcons () const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		void SetIcon (QAction*);
		void FindIconSets ();
		void FindIcons ();
	private slots:
		void flushCaches ();
	};
};

#endif

