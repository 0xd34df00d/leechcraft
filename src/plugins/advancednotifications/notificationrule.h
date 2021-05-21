/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QMetaType>
#include <QColor>
#include "common.h"
#include "fieldmatch.h"
#include "interfaces/advancednotifications/inotificationrule.h"

class QDataStream;
class QDebug;

namespace LC::AdvancedNotifications
{
	struct VisualParams
	{
		bool operator== (const VisualParams&) const = default;
	};

	struct AudioParams
	{
		QString Filename_;

		bool operator== (const AudioParams&) const = default;
	};

	struct TrayParams
	{
		bool operator== (const TrayParams&) const = default;
	};

	struct CmdParams
	{
		QString Cmd_;
		QStringList Args_;

		bool operator== (const CmdParams&) const = default;
	};

	class NotificationRule : public INotificationRule
	{
		QString Name_;
		QString Category_;
		QStringList Types_;

		NotificationMethods Methods_ = NMNone;

		FieldMatches_t FieldMatches_;

		AudioParams AudioParams_;
		TrayParams TrayParams_;
		VisualParams VisualParams_;
		CmdParams CmdParams_;

		bool IsEnabled_ = true;
		bool IsSingleShot_ = false;

		QColor Color_ = Qt::red;
	public:
		NotificationRule () = default;
		NotificationRule (const QString& name, const QString& cat, const QStringList& types);

		bool IsNull () const override;

		QString GetName () const;
		void SetName (const QString&);

		QString GetCategory () const;
		void SetCategory (const QString&);

		QSet<QString> GetTypes () const;
		void SetTypes (const QStringList&);

		NotificationMethods GetMethods () const;
		void SetMethods (NotificationMethods);
		void AddMethod (NotificationMethod);

		FieldMatches_t GetFieldMatches () const;
		void SetFieldMatches (const FieldMatches_t&);
		void AddFieldMatch (const FieldMatch&);

		VisualParams GetVisualParams () const;
		void SetVisualParams (const VisualParams&);

		AudioParams GetAudioParams () const;
		void SetAudioParams (const AudioParams&);

		TrayParams GetTrayParams () const;
		void SetTrayParams (const TrayParams&);

		CmdParams GetCmdParams () const;
		void SetCmdParams (const CmdParams&);

		bool IsEnabled () const;
		void SetEnabled (bool);

		bool IsSingleShot () const;
		void SetSingleShot (bool);

		QColor GetColor () const override;
		void SetColor (const QColor&);

		void Save (QDataStream&) const;
		void Load (QDataStream&);

		bool operator== (const NotificationRule&) const = default;
	};

	void DebugEquals (const NotificationRule&, const NotificationRule&);
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::FieldMatch&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::VisualParams&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::AudioParams&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::TrayParams&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::CmdParams&);

Q_DECLARE_METATYPE (LC::AdvancedNotifications::NotificationRule)
Q_DECLARE_METATYPE (QList<LC::AdvancedNotifications::NotificationRule>)
