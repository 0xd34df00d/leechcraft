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
	typename std::enable_if<std::is_enum<Enum>::value, QByteArray>::type ToMRIM (Enum e)
	{
		return ToMRIM (static_cast<quint32> (e));
	}

	QByteArray ToMRIM ();

	template<typename T, typename... Args>
	QByteArray ToMRIM (T t, Args... args)
	{
		return ToMRIM (t) + ToMRIM (args...);
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

	template<typename Enum>
	void FromMRIM (QByteArray& ba, Enum& out, typename std::enable_if<std::is_enum<Enum>::value, void>::type* = nullptr)
	{
		quint32 proxy = 0;
		FromMRIM (ba, proxy);
		out = static_cast<Enum> (proxy);
	}

	template<typename T, typename... Args>
	void FromMRIM (QByteArray& ba, T& u, Args&... args)
	{
		FromMRIM (ba, u);
		FromMRIM (ba, args...);
	}
}
}
}
}
