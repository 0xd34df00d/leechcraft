/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QColor>
#include <QMap>
#include <QModelIndexList>

class QStandardItemModel;
class QAbstractItemModel;

typedef QList<QVariant> QVariantList;

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	class EventsSettingsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
	public:
		struct EventInfo
		{
			QColor Color_;
		};
	private:
		QMap<QString, EventInfo> EnabledEvents_;
	public:
		EventsSettingsManager (QObject* = nullptr);

		QMap<QString, EventInfo> GetEnabledEvents () const;

		QAbstractItemModel* GetModel () const;
	private:
		void AppendRow (const QString&, const QColor&, bool = true);

		void LoadSettings ();
		void LoadDefaultSettings ();

		void RebuildEnabledEvents ();
		void RebuildAddableEvents ();
	public slots:
		void addRequested (const QString&, const QVariantList&);
		void modifyRequested (const QString&, int, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);
	private slots:
		void saveSettings ();
		void handleItemChanged ();
	signals:
		void eventsSettingsChanged ();
	};
}
}
}
