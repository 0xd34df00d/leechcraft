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

namespace LC
{
namespace AdvancedNotifications
{
	struct VisualParams
	{
	};

	bool operator== (const VisualParams&, const VisualParams&);

	struct AudioParams
	{
		QString Filename_;

		AudioParams ();
		AudioParams (const QString&);
	};

	bool operator== (const AudioParams&, const AudioParams&);

	struct TrayParams
	{
	};

	bool operator== (const TrayParams&, const TrayParams&);

	struct CmdParams
	{
		QString Cmd_;
		QStringList Args_;

		CmdParams ();
		CmdParams (const QString&, const QStringList& = QStringList ());
	};

	bool operator== (const CmdParams&, const CmdParams&);

	class NotificationRule : public INotificationRule
	{
		QString Name_;
		QString Category_;
		QStringList Types_;

		NotificationMethods Methods_;

		FieldMatches_t FieldMatches_;

		AudioParams AudioParams_;
		TrayParams TrayParams_;
		VisualParams VisualParams_;
		CmdParams CmdParams_;

		bool IsEnabled_;
		bool IsSingleShot_;

		QColor Color_ { Qt::red };
	public:
		NotificationRule ();
		NotificationRule (const QString& name,
				const QString& cat, const QStringList& types);

		bool IsNull () const;

		QString GetName () const;
		void SetName (const QString&);

		QString GetCategory () const;
		void SetCategory (const QString&);

		QSet<QString> GetTypes () const;
		void SetTypes (const QStringList&);

		NotificationMethods GetMethods () const;
		void SetMethods (const NotificationMethods&);
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

		QColor GetColor () const;
		void SetColor (const QColor&);

		void Save (QDataStream&) const;
		void Load (QDataStream&);
	};

	bool operator== (const NotificationRule&, const NotificationRule&);
	bool operator!= (const NotificationRule&, const NotificationRule&);

	void DebugEquals (const NotificationRule&, const NotificationRule&);
}
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::FieldMatch&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::VisualParams&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::AudioParams&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::TrayParams&);
QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::CmdParams&);

Q_DECLARE_METATYPE (LC::AdvancedNotifications::NotificationRule)
Q_DECLARE_METATYPE (QList<LC::AdvancedNotifications::NotificationRule>)
