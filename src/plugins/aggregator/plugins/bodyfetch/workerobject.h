/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QHash>
#include <QUrl>
#include <QDir>
#include <interfaces/iscriptloader.h>

namespace LC
{
namespace Aggregator
{
class IProxyObject;

struct Item;

namespace BodyFetch
{
	class WorkerObject : public QObject
	{
		Q_OBJECT

		IProxyObject * const AggregatorProxy_;

		IScriptLoaderInstance_ptr Inst_;
		QList<Item> Items_;

		bool IsProcessing_ = false;
		bool RecheckScheduled_ = false;

		QStringList EnumeratedCache_;

		QHash<QString, QString> ChannelLink2ScriptID_;
		QHash<QUrl, IScript_ptr> URL2Script_;
		QHash<QUrl, quint64> URL2ItemID_;

		QHash<QString, IScript_ptr> CachedScripts_;

		QList<QPair<QUrl, QString>> FetchedQueue_;

		QDir StorageDir_;
	public:
		WorkerObject (IProxyObject*, QObject* = nullptr);

		void SetLoaderInstance (const IScriptLoaderInstance_ptr&);
		bool IsOk () const;
		void AppendItem (const Item&);
	private:
		void ProcessItems (const QList<Item>&);
		IScript_ptr GetScriptForChannel (const QString&);
		QString FindScriptForChannel (const QString&);
		QString Parse (const QString&, IScript_ptr);
		void WriteFile (const QString&, quint64) const;
		QString Recode (const QByteArray&) const;
		void ScheduleRechecking ();
	private slots:
		void handleDownloadFinished (QUrl, QString);
		void recheckFinished ();
		void process ();
		void clearCaches ();
	signals:
		void downloadRequested (QUrl);
		void newBodyFetched (quint64);
	};
}
}
}
