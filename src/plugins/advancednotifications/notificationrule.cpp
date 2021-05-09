/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationrule.h"
#include <QDataStream>
#include <QStringList>
#include <QtDebug>
#include <util/sll/containerconversions.h>

namespace LC
{
namespace AdvancedNotifications
{
	NotificationRule::NotificationRule (const QString& name,
			const QString& cat, const QStringList& types)
	: Name_ (name)
	, Category_ (cat)
	, Types_ (types)
	{
	}

	bool NotificationRule::IsNull () const
	{
		return Name_.isEmpty () ||
				Category_.isEmpty () ||
				Types_.isEmpty ();
	}

	QString NotificationRule::GetName () const
	{
		return Name_;
	}

	void NotificationRule::SetName (const QString& name)
	{
		Name_ = name;
	}

	QString NotificationRule::GetCategory () const
	{
		return Category_;
	}

	void NotificationRule::SetCategory (const QString& cat)
	{
		Category_ = cat;
	}

	QSet<QString> NotificationRule::GetTypes () const
	{
		return Util::AsSet (Types_);
	}

	void NotificationRule::SetTypes (const QStringList& types)
	{
		Types_ = types;
	}

	NotificationMethods NotificationRule::GetMethods () const
	{
		return Methods_;
	}

	void NotificationRule::SetMethods (const NotificationMethods& methods)
	{
		Methods_ = methods;
	}

	void NotificationRule::AddMethod (NotificationMethod method)
	{
		Methods_ |= method;
	}

	FieldMatches_t NotificationRule::GetFieldMatches () const
	{
		return FieldMatches_;
	}

	VisualParams NotificationRule::GetVisualParams () const
	{
		return VisualParams_;
	}

	void NotificationRule::SetVisualParams (const VisualParams& params)
	{
		VisualParams_ = params;
	}

	AudioParams NotificationRule::GetAudioParams () const
	{
		return AudioParams_;
	}

	void NotificationRule::SetAudioParams (const AudioParams& params)
	{
		AudioParams_ = params;
	}

	TrayParams NotificationRule::GetTrayParams () const
	{
		return TrayParams_;
	}

	void NotificationRule::SetTrayParams (const TrayParams& params)
	{
		TrayParams_ = params;
	}

	CmdParams NotificationRule::GetCmdParams() const
	{
		return CmdParams_;
	}

	void NotificationRule::SetCmdParams (const CmdParams& params)
	{
		CmdParams_ = params;
	}

	bool NotificationRule::IsEnabled () const
	{
		return IsEnabled_;
	}

	void NotificationRule::SetEnabled (bool enabled)
	{
		IsEnabled_ = enabled;
	}

	bool NotificationRule::IsSingleShot () const
	{
		return IsSingleShot_;
	}

	void NotificationRule::SetSingleShot (bool shot)
	{
		IsSingleShot_ = shot;
	}

	void NotificationRule::SetFieldMatches (const FieldMatches_t& matches)
	{
		FieldMatches_ = matches;
	}

	void NotificationRule::AddFieldMatch (const FieldMatch& match)
	{
		FieldMatches_ << match;
	}

	QColor NotificationRule::GetColor () const
	{
		return Color_;
	}

	void NotificationRule::SetColor (const QColor& color)
	{
		Color_ = color;
	}

	void NotificationRule::Save (QDataStream& stream) const
	{
		stream << static_cast<quint8> (4)
			<< Name_
			<< Category_
			<< Types_
			<< static_cast<quint16> (Methods_)
			<< AudioParams_.Filename_
			<< CmdParams_.Cmd_
			<< CmdParams_.Args_
			<< IsEnabled_
			<< IsSingleShot_
			<< Color_
			<< static_cast<quint16> (FieldMatches_.size ());

		for (const auto& match : FieldMatches_)
			match.Save (stream);
	}

	void NotificationRule::Load (QDataStream& stream)
	{
		quint8 version = 0;
		stream >> version;
		if (version < 1 || version > 4)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return;
		}

		quint16 methods;
		stream >> Name_
			>> Category_
			>> Types_
			>> methods
			>> AudioParams_.Filename_;

		if (version >= 2)
			stream >> CmdParams_.Cmd_
				>> CmdParams_.Args_;

		if (version >= 3)
			stream >> IsEnabled_
				>> IsSingleShot_;
		else
		{
			IsEnabled_ = true;
			IsSingleShot_ = false;
		}

		if (version >= 4)
			stream >> Color_;

		Methods_ = static_cast<NotificationMethods> (methods);

		quint16 numMatches = 0;
		stream >> numMatches;

		for (int i = 0; i < numMatches; ++i)
		{
			FieldMatch match;
			match.Load (stream);
			FieldMatches_ << match;
		}
	}

	namespace
	{
		template<typename T>
		void DebugSingle (const NotificationRule& r1, const NotificationRule& r2, T method)
		{
			const auto eq = (r1.*method) () == (r2.*method) ();
			qDebug () << eq;
			if (!eq)
				qDebug () << (r1.*method) () << (r2.*method) ();
		}
	}

	void DebugEquals (const NotificationRule& r1, const NotificationRule& r2)
	{
		qDebug () << Q_FUNC_INFO;
		DebugSingle (r1, r2, &NotificationRule::GetMethods);
		DebugSingle (r1, r2, &NotificationRule::IsEnabled);
		DebugSingle (r1, r2, &NotificationRule::IsSingleShot);
		DebugSingle (r1, r2, &NotificationRule::GetName);
		DebugSingle (r1, r2, &NotificationRule::GetCategory);
		DebugSingle (r1, r2, &NotificationRule::GetTypes);
		DebugSingle (r1, r2, &NotificationRule::GetFieldMatches);
		DebugSingle (r1, r2, &NotificationRule::GetVisualParams);
		DebugSingle (r1, r2, &NotificationRule::GetAudioParams);
		DebugSingle (r1, r2, &NotificationRule::GetTrayParams);
		DebugSingle (r1, r2, &NotificationRule::GetCmdParams);
		DebugSingle (r1, r2, &NotificationRule::GetColor);
	}
}
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::FieldMatch& match)
{
	dbg.nospace () << "FieldMatch (for: " << match.GetPluginID () << "; field: " << match.GetFieldName () << ")";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::VisualParams&)
{
	dbg.nospace () << "VisualParams ()";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::AudioParams& params)
{
	dbg.nospace () << "AudioParams (file: " << params.Filename_ << ")";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::TrayParams&)
{
	dbg.nospace () << "TrayParams ()";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LC::AdvancedNotifications::CmdParams& params)
{
	dbg.nospace () << "CmdParams (command: " << params.Cmd_ << "; args: " << params.Args_ << ")";
	return dbg.space ();
}
