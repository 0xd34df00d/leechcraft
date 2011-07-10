/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_PLUGINS_BODYFETCH_WORKEROBJECT_H
#define PLUGINS_AGGREGATOR_PLUGINS_BODYFETCH_WORKEROBJECT_H
#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QDateTime>
#include <QHash>
#include <QUrl>
#include <QDir>
#include <interfaces/iscriptloader.h>
#include <plugininterface/guarded.h>

namespace LeechCraft
{
namespace Aggregator
{
namespace BodyFetch
{
	class WorkerObject : public QObject
	{
		Q_OBJECT
		
		Util::Guarded<IScriptLoaderInstance*> Inst_;
		Util::Guarded<QVariantList> Items_;
		
		QStringList EnumeratedCache_;
		QDateTime LastEnumerated_;

		QHash<QString, QString> ChannelLink2ScriptID_;
		QHash<QUrl, IScript_ptr> URL2Script_;
		QHash<QUrl, quint64> URL2ItemID_;
		
		QDir StorageDir_;
	public:
		WorkerObject (QObject* = 0);
		
		void SetLoaderInstance (IScriptLoaderInstance*);
		void AppendItems (const QVariantList&);
	private:
		void ProcessItems (const QVariantList&);
		IScript_ptr GetScriptForChannel (const QString&);
		QString FindScriptForChannel (const QString&);
		QString Parse (const QString&, IScript_ptr);
		QString ParseWithSelectors (const QString&, const QStringList&);
		void WriteFile (const QString&, quint64) const;
		QString Recode (const QByteArray&) const;
	public slots:
		void handleDownloadFinished (QUrl, QString);
		void process ();
	signals:
		void downloadRequested (QUrl);
		void newBodyFetched (quint64);
	};
}
}
}

#endif
