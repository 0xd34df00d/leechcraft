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
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <util/xpc/anutil.h>
#include "ui_boolmatcherconfigwidget.h"
#include "ui_intmatcherconfigwidget.h"
#include "ui_stringlikematcherconfigwidget.h"

namespace LC::AdvancedNotifications
{
	TypedMatcherBase_ptr TypedMatcherBase::Create (QMetaType::Type type, const AN::FieldData& fieldData)
	{
		switch (type)
		{
		case QMetaType::Bool:
			return std::make_shared<BoolMatcher> (fieldData.Name_);
		case QMetaType::Int:
			return std::make_shared<IntMatcher> ();
		case QMetaType::QString:
			return std::make_shared<StringMatcher> (fieldData.AllowedValues_);
		case QMetaType::QStringList:
			return std::make_shared<StringListMatcher> (fieldData.AllowedValues_);
		case QMetaType::QUrl:
			return std::make_shared<UrlMatcher> ();
		default:
			qWarning () << "unknown type" << type;
			return {};
		}
	}

	StringLikeMatcher::StringLikeMatcher (const QList<AN::FieldData::AllowedValue>& variants)
	: AllowedValues_ { variants }
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
			{ Keys::Rx, Util::AN::ToVariant (Value_.Pattern_) },
			{ Keys::Contains, Value_.Positive_ }
		};
	}

	void StringLikeMatcher::Load (const QVariantMap& map)
	{
		if (const auto sm = Util::AN::StringPatternFromVariant (map [Keys::Rx]))
		{
			Value_.Pattern_ = *sm;
			Value_.Positive_ = map [Keys::Contains].toBool ();
		}
		else
			qWarning () << "cannot load string matcher from variant" << map;
	}

	namespace
	{
		template<typename T>
		void SetValueFromVariant (T& value, const AN::ValueMatcher& variant)
		{
			Util::Visit (variant,
					[&value] (const T& val) { value = val; },
					[] (const auto&) {});
		}
	}

	void StringLikeMatcher::SetValue (const AN::ValueMatcher& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	using SVM = AN::StringValueMatcher;

	void StringLikeMatcher::SetValue (const QVariant& variant)
	{
		if (const auto em = get_if<SVM::Exact> (&variant))
			Value_.Pattern_ = *em;
		else if (const auto str = get_if<QString> (&variant))
			Value_.Pattern_ = *str;
		else if (const auto ss = get_if<SVM::Substring> (&variant))
			Value_.Pattern_ = *ss;
		else if (const auto wc = get_if<SVM::Wildcard> (&variant))
			Value_.Pattern_ = *wc;
		else if (const auto rx = get_if<QRegularExpression> (&variant))
			Value_.Pattern_ = *rx;
		else
		{
			qWarning () << "unsupported type:" << variant;
			throw std::runtime_error { "unsupported type" };
		}

		Value_.Positive_ = true;
	}

	AN::ValueMatcher StringLikeMatcher::GetValue () const
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

			if (AllowedValues_.isEmpty ())
				Ui_->VariantsBox_->hide ();
			else
			{
				for (const auto& allowed : AllowedValues_)
					Ui_->VariantsBox_->addItem (allowed.Name_, allowed.Id_);
				Ui_->RegexType_->hide ();
				Ui_->RegexpEditor_->hide ();
			}
		}

		SyncWidgetTo ();

		return CW_;
	}

	namespace
	{
		using UiOrderedStringMatcher = std::variant<SVM::Exact, SVM::Substring, SVM::Wildcard, QRegularExpression>;

		template<size_t Idx>
		using UiOrderedAlternative = std::variant_alternative_t<Idx, UiOrderedStringMatcher>;

		static_assert (std::variant_size_v<UiOrderedStringMatcher> == std::variant_size_v<SVM::Pattern::Base>);

		int ToUiIndex (const SVM::Pattern& matcher)
		{
			return Util::Visit (matcher, [] (const auto& val) { return UiOrderedStringMatcher { val }.index (); });
		}

		QString ExtractPattern (const SVM::Pattern& matcher)
		{
			return Util::Visit (matcher,
					[] (const QRegularExpression& rx) { return rx.pattern (); },
					[] (const auto& val) { return val.Pattern_; });
		}

		std::optional<SVM::Pattern> FromUiIndex (int index, const QString& pattern)
		{
			return [&]<size_t... Idx> (std::index_sequence<Idx...>)
			{
				// TODO C++26 template for
				std::optional<SVM::Pattern> result;
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
		if (AllowedValues_.isEmpty ())
		{
			const auto idx = Ui_->RegexType_->currentIndex ();
			if (const auto matcher = FromUiIndex (idx, Ui_->RegexpEditor_->text ()))
				Value_.Pattern_ = *matcher;
			else
				qWarning () << "unknown string matcher type" << idx;
		}
		else
			Value_.Pattern_ = SVM::Exact { Ui_->VariantsBox_->currentData ().toByteArray () };
	}

	void StringLikeMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << "called with null CW";
			return;
		}

		Ui_->ContainsBox_->setCurrentIndex (!Value_.Positive_);
		if (AllowedValues_.isEmpty ())
		{
			Ui_->RegexpEditor_->setText (ExtractPattern (Value_.Pattern_));
			Ui_->RegexType_->setCurrentIndex (ToUiIndex (Value_.Pattern_));
		}
		else
		{
			const auto& pattern = ExtractPattern (Value_.Pattern_).toUtf8 ();
			if (const auto idx = Ui_->VariantsBox_->findData (pattern);
				idx >= 0)
				Ui_->VariantsBox_->setCurrentIndex (idx);
			else
				qWarning () << "cannot find" << pattern << "in" << Util::Map (AllowedValues_, &AN::FieldData::AllowedValue::Id_);
		}
	}

	namespace
	{
		bool GenericMatch (auto&& val, const AN::StringValueMatcher& ref)
		{
			return Util::AN::Matches (val, ref.Pattern_) == ref.Positive_;
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
			static QString ForStringMatcher (const AN::StringValueMatcher& value)
			{
				const auto contains = value.Positive_;
				return Util::Visit (value.Pattern_,
						[&] (const QRegularExpression& rx)
						{
							const auto& msg = contains ?
									tr ("matches regular expression `%1`") :
									tr ("doesn't match regular expression `%1`");
							return msg.arg (rx.pattern ());
						},
						[&] (const SVM::Substring& str)
						{
							const auto& msg = contains ?
									tr ("matches substring `%1`") :
									tr ("doesn't match substring `%1`");
							return msg.arg (str.Pattern_);
						},
						[&] (const SVM::Wildcard& wc)
						{
							const auto& msg = contains ?
									tr ("matches wildcard `%1`") :
									tr ("doesn't match wildcard `%1`");
							return msg.arg (wc.Pattern_);
						},
						[&] (const SVM::Exact& em)
						{
							const auto& msg = contains ?
									tr ("is exactly `%1`") :
									tr ("isn't exactly `%1`");
							return msg.arg (em.Pattern_);
						});
			}

			static QString ForStringListMatcher (const AN::StringValueMatcher& value)
			{
				const auto contains = value.Positive_;
				return Util::Visit (value.Pattern_,
						[&] (const QRegularExpression& rx)
						{
							const auto& msg = contains ?
									tr ("contains a string matching regular expression `%1`") :
									tr ("doesn't contains a string matching regular expression `%1`");
							return msg.arg (rx.pattern ());
						},
						[&] (const SVM::Substring& str)
						{
							const auto& msg = contains ?
									tr ("contains a string with the substring `%1`") :
									tr ("doesn't contain a string with the substring `%1`");
							return msg.arg (str.Pattern_);
						},
						[&] (const SVM::Wildcard& wc)
						{
							const auto& msg = contains ?
									tr ("contains a string matching wildcard `%1`") :
									tr ("doesn't contain a string matching wildcard `%1`");
							return msg.arg (wc.Pattern_);
						},
						[&] (const SVM::Exact& em)
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
		const auto contains = Util::AN::Matches (url.toString (), Value_.Pattern_) ||
				Util::AN::Matches (QString::fromUtf8 (url.toEncoded ()), Value_.Pattern_);
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
		return { { Keys::IsSet, Value_.Value_ } };
	}

	void BoolMatcher::Load (const QVariantMap& map)
	{
		Value_.Value_ = map.value (Keys::IsSet).toBool ();
	}

	void BoolMatcher::SetValue (const AN::ValueMatcher& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void BoolMatcher::SetValue (const QVariant& variant)
	{
		Value_.Value_ = variant.toBool ();
	}

	AN::ValueMatcher BoolMatcher::GetValue () const
	{
		return Value_;
	}

	bool BoolMatcher::Match (const QVariant& var) const
	{
		return var.toBool () == Value_.Value_;
	}

	QString BoolMatcher::GetHRDescription () const
	{
		return Value_.Value_ ?
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

		Value_.Value_ = Ui_->IsSet_->checkState () == Qt::Checked;
	}

	void BoolMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Ui_->IsSet_->setCheckState (Value_.Value_ ? Qt::Checked : Qt::Unchecked);
	}

	IntMatcher::IntMatcher ()
	{
		Ops2pos_ [AN::IntValueMatcher::OGreater] = 0;
		Ops2pos_ [AN::IntValueMatcher::OEqual | AN::IntValueMatcher::OGreater] = 1;
		Ops2pos_ [AN::IntValueMatcher::OEqual] = 2;
		Ops2pos_ [AN::IntValueMatcher::OEqual | AN::IntValueMatcher::OLess] = 3;
		Ops2pos_ [AN::IntValueMatcher::OLess] = 4;
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
		Value_.Ops_ = static_cast<AN::IntValueMatcher::Operations> (map [Keys::Ops].value<quint16> ());
	}

	void IntMatcher::SetValue (const AN::ValueMatcher& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void IntMatcher::SetValue (const QVariant& variant)
	{
		Value_.Boundary_ = variant.toInt ();
		Value_.Ops_ = AN::IntValueMatcher::OEqual;
	}

	AN::ValueMatcher IntMatcher::GetValue () const
	{
		return Value_;
	}

	bool IntMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<int> ())
			return false;

		const int val = var.toInt ();

		if ((Value_.Ops_ & AN::IntValueMatcher::OEqual) && val == Value_.Boundary_)
			return true;
		if ((Value_.Ops_ & AN::IntValueMatcher::OGreater) && val > Value_.Boundary_)
			return true;
		if ((Value_.Ops_ & AN::IntValueMatcher::OLess) && val < Value_.Boundary_)
			return true;

		return false;
	}

	QString IntMatcher::GetHRDescription () const
	{
		if (Value_.Ops_ == AN::IntValueMatcher::OEqual)
			return QObject::tr ("equals to %1").arg (Value_.Boundary_);

		QString op;
		if ((Value_.Ops_ & AN::IntValueMatcher::OGreater))
			op += ">"_ql;
		if ((Value_.Ops_ & AN::IntValueMatcher::OLess))
			op += "<"_ql;
		if ((Value_.Ops_ & AN::IntValueMatcher::OEqual))
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
