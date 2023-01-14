/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QByteArray>
#include <QTextCodec>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class UIDL
	{
		QByteArray ID_;
	public:
		UIDL ();
		UIDL (const UIDL&);
		explicit UIDL (const QByteArray&);

		UIDL& operator= (const QByteArray&);

		operator QByteArray () const;
	};

	QByteArray ToMRIM1251 (const QString&);
	QByteArray ToMRIM16 (const QString&);
	QByteArray ToMRIM (const QString&);
	QByteArray ToMRIM (const QByteArray&);
	QByteArray ToMRIM (const UIDL&);
	QByteArray ToMRIM (quint32);
	QByteArray ToMRIM (int);

	template<typename Enum>
	concept IsEnum = std::is_enum_v<Enum>;

	template<IsEnum Enum>
	QByteArray ToMRIM (Enum e)
	{
		return ToMRIM (static_cast<quint32> (e));
	}

	QByteArray ToMRIM ();

	template<typename... Args>
		requires (sizeof... (Args) > 0)
	QByteArray ToMRIM (Args... args)
	{
		return (ToMRIM (args) + ...);
	}

	struct EncoderProxy
	{
		QString Str_;

		EncoderProxy& operator= (const QByteArray& ba)
		{
			Str_ = QTextCodec::codecForName (GetCodecName ())->toUnicode (ba);
			return *this;
		}

		operator QString () const { return Str_; }
	protected:
		virtual QByteArray GetCodecName () = 0;
	};

	struct Str1251 : EncoderProxy
	{
	protected:
		QByteArray GetCodecName () { return "Windows-1251"; }
	};

	struct Str16 : EncoderProxy
	{
	protected:
		QByteArray GetCodecName () { return "UTF-16LE"; }
	};

	QString FromMRIM1251 (const QByteArray&);
	QString FromMRIM16 (const QByteArray&);
	void FromMRIM (QByteArray&, EncoderProxy&);
	inline void FromMRIM (QByteArray& ba, Str1251& str) { FromMRIM (ba, static_cast<EncoderProxy&> (str)); }
	inline void FromMRIM (QByteArray& ba, Str16& str){ FromMRIM (ba, static_cast<EncoderProxy&> (str)); }
	void FromMRIM (QByteArray&, UIDL&);
	void FromMRIM (QByteArray&, QByteArray&);
	void FromMRIM (QByteArray&, quint32&);
	void FromMRIM (QByteArray&);

	template<IsEnum Enum>
	void FromMRIM (QByteArray& ba, Enum& out)
	{
		quint32 proxy = 0;
		FromMRIM (ba, proxy);
		out = static_cast<Enum> (proxy);
	}

	template<typename... Args>
		requires (sizeof... (Args) > 0)
	void FromMRIM (QByteArray& ba, Args&... args)
	{
		(FromMRIM (ba, args), ...);
	}
}
}
}
}
