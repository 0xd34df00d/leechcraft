/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typedmatchers.h"
#include <QStringList>
#include <QWidget>
#include <QtDebug>
#include <QUrl>
#include <util/sll/demangle.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <util/xpc/anutil.h>
#include "ui_boolmatcherconfigwidget.h"
#include "ui_intmatcherconfigwidget.h"
#include "ui_stringlikematcherconfigwidget.h"

namespace LC::AdvancedNotifications
{
	namespace
	{
		template<typename T>
		QList<T> ToTList (const QVariantList& list)
		{
			QList<T> result;
			for (const auto& item : list)
				if (item.canConvert<T> ())
					result << item.value<T> ();
				else
					qWarning () << "cannot convert" << item << "to" << Util::Demangle (typeid (T));
			return result;
		}
	}

	TypedMatcherBase_ptr TypedMatcherBase::Create (QMetaType::Type type, const AN::FieldData& fieldData)
	{
		switch (type)
		{
		case QMetaType::Bool:
			return std::make_shared<BoolMatcher> (fieldData.Name_);
		case QMetaType::Int:
			return std::make_shared<IntMatcher> ();
		case QMetaType::QString:
			return std::make_shared<StringMatcher> (ToTList<QString> (fieldData.AllowedValues_));
		case QMetaType::QStringList:
			return std::make_shared<StringListMatcher> (ToTList<QString> (fieldData.AllowedValues_));
		case QMetaType::QUrl:
			return std::make_shared<UrlMatcher> ();
		default:
			qWarning () << "unknown type" << type;
			return {};
		}
	}

	StringLikeMatcher::StringLikeMatcher (const QStringList& variants)
	: Allowed_ (variants)
	{
	}

	namespace Keys
	{
		const QString Rx = QStringLiteral ("Rx");
		const QString Contains = QStringLiteral ("Cont");
	}

	QVariantMap StringLikeMatcher::Save () const
	{
		return
		{
			{ Keys::Rx, Util::AN::ToVariant (Value_.Matcher_) },
			{ Keys::Contains, Value_.Positive_ }
		};
	}

	void StringLikeMatcher::Load (const QVariantMap& map)
	{
		if (const auto sm = Util::AN::StringMatcherFromVariant (map [Keys::Rx]))
		{
			Value_.Matcher_ = *sm;
			Value_.Positive_ = map [Keys::Contains].toBool ();
		}
		else
			qWarning () << "cannot load string matcher from variant" << map;
	}

	namespace
	{
		template<typename T>
		void SetValueFromVariant (T& value, const AN::FieldValue& variant)
		{
			Util::Visit (variant,
					[&value] (const T& val) { value = val; },
					[] (const auto&) {});
		}
	}

	void StringLikeMatcher::SetValue (const AN::FieldValue& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void StringLikeMatcher::SetValue (const QVariant& variant)
	{
		if (const auto em = get_if<AN::ExactMatch> (&variant))
			Value_.Matcher_ = *em;
		else if (const auto str = get_if<QString> (&variant))
			Value_.Matcher_ = *str;
		else if (const auto ss = get_if<AN::Substring> (&variant))
			Value_.Matcher_ = *ss;
		else if (const auto wc = get_if<AN::Wildcard> (&variant))
			Value_.Matcher_ = *wc;
		else if (const auto rx = get_if<QRegularExpression> (&variant))
			Value_.Matcher_ = *rx;
		else
		{
			qWarning () << "unsupported type:" << variant;
			throw std::runtime_error { "unsupported type" };
		}

		Value_.Positive_ = true;
	}

	AN::FieldValue StringLikeMatcher::GetValue () const
	{
		return Value_;
	}

	QWidget* StringLikeMatcher::GetConfigWidget ()
	{
		if (!CW_)
		{
			CW_ = new QWidget ();
			Ui_.reset (new Ui::StringLikeMatcherConfigWidget ());
			Ui_->setupUi (CW_);

			if (Allowed_.isEmpty ())
				Ui_->VariantsBox_->hide ();
			else
			{
				Ui_->VariantsBox_->addItems (Allowed_);
				Ui_->RegexType_->hide ();
				Ui_->RegexpEditor_->hide ();
			}
		}

		SyncWidgetTo ();

		return CW_;
	}

	namespace
	{
		using UiOrderedStringMatcher = std::variant<AN::ExactMatch, AN::Substring, AN::Wildcard, QRegularExpression>;

		template<size_t Idx>
		using UiOrderedAlternative = std::variant_alternative_t<Idx, UiOrderedStringMatcher>;

		static_assert (std::variant_size_v<UiOrderedStringMatcher> == std::variant_size_v<AN::StringMatcher::Base>);

		int ToUiIndex (const AN::StringMatcher& matcher)
		{
			return Util::Visit (matcher, [] (const auto& val) { return UiOrderedStringMatcher { val }.index (); });
		}

		QString ExtractPattern (const AN::StringMatcher& matcher)
		{
			return Util::Visit (matcher,
					[] (const QRegularExpression& rx) { return rx.pattern (); },
					[] (const auto& val) { return val.Pattern_; });
		}

		std::optional<AN::StringMatcher> FromUiIndex (int index, const QString& pattern)
		{
			return [&]<size_t... Idx> (std::index_sequence<Idx...>)
			{
				// TODO C++26 template for
				std::optional<AN::StringMatcher> result;
				static_cast<void> (((index == Idx && (result = UiOrderedAlternative<Idx> { pattern }, 0)) + ...));
				return result;
			} (std::make_index_sequence<std::variant_size_v<UiOrderedStringMatcher>> {});
		}
	}

	void StringLikeMatcher::SyncToWidget ()
	{
		if (!CW_)
		{
			qWarning () << "called with null CW";
			return;
		}

		Value_.Positive_ = Ui_->ContainsBox_->currentIndex () == 0;
		if (Allowed_.isEmpty ())
		{
			const auto idx = Ui_->RegexType_->currentIndex ();
			if (const auto matcher = FromUiIndex (idx, Ui_->RegexpEditor_->text ()))
				Value_.Matcher_ = *matcher;
			else
				qWarning () << "unknown string matcher type" << idx;
		}
		else
			Value_.Matcher_ = AN::ExactMatch { Ui_->VariantsBox_->currentText () };
	}

	void StringLikeMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << "called with null CW";
			return;
		}

		Ui_->ContainsBox_->setCurrentIndex (!Value_.Positive_);
		if (Allowed_.isEmpty ())
		{
			Ui_->RegexpEditor_->setText (ExtractPattern (Value_.Matcher_));
			Ui_->RegexType_->setCurrentIndex (ToUiIndex (Value_.Matcher_));
		}
		else
		{
			const auto& pattern = ExtractPattern (Value_.Matcher_);
			if (const auto idx = Ui_->VariantsBox_->findText (pattern);
				idx >= 0)
				Ui_->VariantsBox_->setCurrentIndex (idx);
			else
				qWarning () << "cannot find" << pattern << "in" << Allowed_;
		}
	}

	namespace
	{
		bool GenericMatch (auto&& val, const AN::StringFieldValue& ref)
		{
			return Util::AN::Matches (val, ref.Matcher_) == ref.Positive_;
		}
	}

	bool StringMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<QString> ())
			return false;
		return GenericMatch (var.toString (), Value_);
	}

	namespace
	{
		struct Descriptions
		{
			Q_DECLARE_TR_FUNCTIONS (LC::AdvancedNotifications::Descriptions)
		public:
			static QString ForStringMatcher (const AN::StringFieldValue& value)
			{
				const auto contains = value.Positive_;
				return Util::Visit (value.Matcher_,
						[&] (const QRegularExpression& rx)
						{
							const auto& msg = contains ?
									tr ("matches regular expression `%1`") :
									tr ("doesn't match regular expression `%1`");
							return msg.arg (rx.pattern ());
						},
						[&] (const AN::Substring& str)
						{
							const auto& msg = contains ?
									tr ("matches substring `%1`") :
									tr ("doesn't match substring `%1`");
							return msg.arg (str.Pattern_);
						},
						[&] (const AN::Wildcard& wc)
						{
							const auto& msg = contains ?
									tr ("matches wildcard `%1`") :
									tr ("doesn't match wildcard `%1`");
							return msg.arg (wc.Pattern_);
						},
						[&] (const AN::ExactMatch& em)
						{
							const auto& msg = contains ?
									tr ("is exactly `%1`") :
									tr ("isn't exactly `%1`");
							return msg.arg (em.Pattern_);
						});
			}

			static QString ForStringListMatcher (const AN::StringFieldValue& value)
			{
				const auto contains = value.Positive_;
				return Util::Visit (value.Matcher_,
						[&] (const QRegularExpression& rx)
						{
							const auto& msg = contains ?
									tr ("contains a string matching regular expression `%1`") :
									tr ("doesn't contains a string matching regular expression `%1`");
							return msg.arg (rx.pattern ());
						},
						[&] (const AN::Substring& str)
						{
							const auto& msg = contains ?
									tr ("contains a string with the substring `%1`") :
									tr ("doesn't contain a string with the substring `%1`");
							return msg.arg (str.Pattern_);
						},
						[&] (const AN::Wildcard& wc)
						{
							const auto& msg = contains ?
									tr ("contains a string matching wildcard `%1`") :
									tr ("doesn't contain a string matching wildcard `%1`");
							return msg.arg (wc.Pattern_);
						},
						[&] (const AN::ExactMatch& em)
						{
							const auto& msg = contains ?
									tr ("contains the exact string `%1`") :
									tr ("doesn't contain the exact string `%1`");
							return msg.arg (em.Pattern_);
						});
			}
		};
	}

	QString StringMatcher::GetHRDescription () const
	{
		return Descriptions::ForStringMatcher (Value_);
	}

	StringListMatcher::StringListMatcher (const QStringList& list)
	: StringLikeMatcher (list)
	{
	}

	bool StringListMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<QStringList> ())
			return false;
		return GenericMatch (var.toStringList (), Value_);
	}

	QString StringListMatcher::GetHRDescription () const
	{
		return Descriptions::ForStringListMatcher (Value_);
	}

	bool UrlMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<QUrl> ())
			return false;

		const auto& url = var.toUrl ();
		const auto contains = Util::AN::Matches (url.toString (), Value_.Matcher_) ||
				Util::AN::Matches (QString::fromUtf8 (url.toEncoded ()), Value_.Matcher_);
		return contains == Value_.Positive_;
	}

	QString UrlMatcher::GetHRDescription () const
	{
		return Descriptions::ForStringMatcher (Value_);
	}

	BoolMatcher::BoolMatcher (const QString& fieldName)
	: FieldName_ { fieldName }
	{
	}

	namespace Keys
	{
		const QString IsSet = QStringLiteral ("IsSet");
	}

	QVariantMap BoolMatcher::Save () const
	{
		return { { Keys::IsSet, Value_.IsSet_ } };
	}

	void BoolMatcher::Load (const QVariantMap& map)
	{
		Value_.IsSet_ = map.value (Keys::IsSet).toBool ();
	}

	void BoolMatcher::SetValue (const AN::FieldValue& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void BoolMatcher::SetValue (const QVariant& variant)
	{
		Value_.IsSet_ = variant.toBool ();
	}

	AN::FieldValue BoolMatcher::GetValue () const
	{
		return Value_;
	}

	bool BoolMatcher::Match (const QVariant& var) const
	{
		return var.toBool () == Value_.IsSet_;
	}

	QString BoolMatcher::GetHRDescription () const
	{
		return Value_.IsSet_ ?
				QObject::tr ("yes") :
				QObject::tr ("no");
	}

	QWidget* BoolMatcher::GetConfigWidget ()
	{
		if (!CW_)
		{
			CW_ = new QWidget ();
			Ui_.reset (new Ui::BoolMatcherConfigWidget ());
			Ui_->setupUi (CW_);
			Ui_->IsSet_->setText (FieldName_);
		}

		SyncWidgetTo ();

		return CW_;
	}

	void BoolMatcher::SyncToWidget ()
	{
		if (!CW_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Value_.IsSet_ = Ui_->IsSet_->checkState () == Qt::Checked;
	}

	void BoolMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Ui_->IsSet_->setCheckState (Value_.IsSet_ ? Qt::Checked : Qt::Unchecked);
	}

	IntMatcher::IntMatcher ()
	{
		Ops2pos_ [AN::IntFieldValue::OGreater] = 0;
		Ops2pos_ [AN::IntFieldValue::OEqual | AN::IntFieldValue::OGreater] = 1;
		Ops2pos_ [AN::IntFieldValue::OEqual] = 2;
		Ops2pos_ [AN::IntFieldValue::OEqual | AN::IntFieldValue::OLess] = 3;
		Ops2pos_ [AN::IntFieldValue::OLess] = 4;
	}

	namespace Keys
	{
		const QString Boundary = QStringLiteral ("Bd");
		const QString Ops = QStringLiteral ("Ops");
	}

	QVariantMap IntMatcher::Save () const
	{
		return
		{
			{ Keys::Boundary, Value_.Boundary_ },
			{ Keys::Ops, static_cast<quint16> (Value_.Ops_) },
		};
	}

	void IntMatcher::Load (const QVariantMap& map)
	{
		Value_.Boundary_ = map [Keys::Boundary].toInt ();
		Value_.Ops_ = static_cast<AN::IntFieldValue::Operations> (map [Keys::Ops].value<quint16> ());
	}

	void IntMatcher::SetValue (const AN::FieldValue& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void IntMatcher::SetValue (const QVariant& variant)
	{
		Value_.Boundary_ = variant.toInt ();
		Value_.Ops_ = AN::IntFieldValue::OEqual;
	}

	AN::FieldValue IntMatcher::GetValue () const
	{
		return Value_;
	}

	bool IntMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<int> ())
			return false;

		const int val = var.toInt ();

		if ((Value_.Ops_ & AN::IntFieldValue::OEqual) && val == Value_.Boundary_)
			return true;
		if ((Value_.Ops_ & AN::IntFieldValue::OGreater) && val > Value_.Boundary_)
			return true;
		if ((Value_.Ops_ & AN::IntFieldValue::OLess) && val < Value_.Boundary_)
			return true;

		return false;
	}

	QString IntMatcher::GetHRDescription () const
	{
		if (Value_.Ops_ == AN::IntFieldValue::OEqual)
			return QObject::tr ("equals to %1").arg (Value_.Boundary_);

		QString op;
		if ((Value_.Ops_ & AN::IntFieldValue::OGreater))
			op += ">"_ql;
		if ((Value_.Ops_ & AN::IntFieldValue::OLess))
			op += "<"_ql;
		if ((Value_.Ops_ & AN::IntFieldValue::OEqual))
			op += "="_ql;

		return QObject::tr ("is %1 then %2")
				.arg (op)
				.arg (Value_.Boundary_);
	}

	QWidget* IntMatcher::GetConfigWidget ()
	{
		if (!CW_)
		{
			CW_ = new QWidget ();
			Ui_.reset (new Ui::IntMatcherConfigWidget ());
			Ui_->setupUi (CW_);
		}

		SyncWidgetTo ();

		return CW_;
	}

	void IntMatcher::SyncToWidget ()
	{
		if (!CW_)
		{
			qWarning () << "called with null CW";
			return;
		}

		Value_.Boundary_  = Ui_->Boundary_->value ();
		Value_.Ops_ = Ops2pos_.key (Ui_->OpType_->currentIndex ());
	}

	void IntMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << "called with null CW";
			return;
		}

		Ui_->Boundary_->setValue (Value_.Boundary_);
		Ui_->OpType_->setCurrentIndex (Ops2pos_ [Value_.Ops_]);
	}
}
