/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "matchconfigwidget.h"
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
	namespace
	{
		class ConfigWidgetBase
			: public QWidget
			, public IConfigWidget
		{
		public:
			QWidget& GetWidget () final
			{
				return *this;
			}
		};

		template<typename Widget, typename... Args>
		IConfigWidget_ptr CreateTypedConfigWidget (const std::optional<AN::ValueMatcher>& matcher, Args&&... args)
		{
			using MatcherType = Widget::MatcherType;

			if (!matcher)
				return std::make_shared<Widget> (std::optional<MatcherType> {}, std::forward<Args> (args)...);

			if (const auto specific = std::get_if<MatcherType> (&*matcher))
				return std::make_shared<Widget> (*specific, std::forward<Args> (args)...);
			qCritical () << "matcher mismatch";
			return {};
		}

		class BoolConfig : public ConfigWidgetBase
		{
			Ui::BoolMatcherConfigWidget Ui_;
		public:
			using MatcherType = AN::BoolValueMatcher;

			explicit BoolConfig (const std::optional<MatcherType>& matcher, const QString& fieldName)
			{
				Ui_.setupUi (this);
				Ui_.IsSet_->setText (fieldName);
				if (matcher)
					Ui_.IsSet_->setChecked (matcher->Value_);
			}

			AN::ValueMatcher GetConfiguredMatcher () const override
			{
				return MatcherType { Ui_.IsSet_->checkState () == Qt::Checked };
			}
		};

		class IntConfig : public ConfigWidgetBase
		{
			Ui::IntMatcherConfigWidget Ui_;

			using enum AN::IntValueMatcher::Operation;

			constexpr static auto UiOps = std::to_array<AN::IntValueMatcher::Operations> ({ OGreater, OEqual | OGreater, OEqual, OEqual | OLess, OLess });
		public:
			using MatcherType = AN::IntValueMatcher;

			explicit IntConfig (const std::optional<MatcherType>& matcher)
			{
				Ui_.setupUi (this);
				if (matcher)
				{
					Ui_.Boundary_->setValue (matcher->Boundary_);
					Ui_.OpType_->setCurrentIndex (Op2Pos (matcher->Ops_));
				}
			}

			AN::ValueMatcher GetConfiguredMatcher () const override
			{
				return MatcherType { .Boundary_ = Ui_.Boundary_->value (), .Ops_ = Pos2Op (Ui_.OpType_->currentIndex ()) };
			}
		private:
			static int Op2Pos (AN::IntValueMatcher::Operations op)
			{
				const auto pos = std::ranges::find (UiOps, op);
				return pos == UiOps.end () ? -1 : pos - UiOps.begin ();
			}

			static AN::IntValueMatcher::Operations Pos2Op (int pos)
			{
				if (pos < 0 || pos >= static_cast<int> (UiOps.size ()))
				{
					qWarning () << "invalid position" << pos;
					return {};
				}
				return UiOps [pos];
			}
		};

		class StringConfig : public ConfigWidgetBase
		{
			Ui::StringLikeMatcherConfigWidget Ui_;
			const bool HasAllowedValues_;
		public:
			using MatcherType = AN::StringValueMatcher;

			explicit StringConfig (const std::optional<MatcherType>& matcher, const std::optional<AN::FieldData::AllowedValues>& allowedValues = {})
			: HasAllowedValues_ { allowedValues && !allowedValues->isEmpty () }
			{
				Ui_.setupUi (this);

				if (HasAllowedValues_)
				{
					for (const auto& allowed : *allowedValues)
						Ui_.VariantsBox_->addItem (allowed.Name_, allowed.Id_);
					Ui_.RegexType_->hide ();
					Ui_.RegexpEditor_->hide ();
				}
				else
				{
					Ui_.VariantsBox_->hide ();
				}

				if (matcher)
				{
					Ui_.ContainsBox_->setCurrentIndex (!matcher->Positive_);
					if (HasAllowedValues_)
					{
						const auto& pattern = ExtractPattern (matcher->Pattern_).toUtf8 ();
						if (const auto idx = Ui_.VariantsBox_->findData (pattern);
							idx >= 0)
							Ui_.VariantsBox_->setCurrentIndex (idx);
						else
							qWarning () << "cannot find" << pattern << "in"
									<< Util::Map (*allowedValues, &AN::FieldData::AllowedValue::Id_);
					}
					else
					{
						Ui_.RegexpEditor_->setText (ExtractPattern (matcher->Pattern_));
						Ui_.RegexType_->setCurrentIndex (ToUiIndex (matcher->Pattern_));
					}
				}
			}

			AN::ValueMatcher GetConfiguredMatcher () const override
			{
				return AN::StringValueMatcher { .Pattern_ = GetPattern (), .Positive_ = Ui_.ContainsBox_->currentIndex () == 0 };
			}
		private:
			using SVM = AN::StringValueMatcher;

			SVM::Pattern GetPattern () const
			{
				if (HasAllowedValues_)
					return SVM::Exact { Ui_.VariantsBox_->currentData ().toByteArray () };

				const auto idx = Ui_.RegexType_->currentIndex ();
				if (const auto matcher = FromUiIndex (idx, Ui_.RegexpEditor_->text ()))
					return *matcher;
				qWarning () << "unknown string matcher type" << idx;
				return {};
			}

			using UiOrderedStringMatcher = std::variant<SVM::Exact, SVM::Substring, SVM::Wildcard, QRegularExpression>;

			template<size_t Idx>
			using UiOrderedAlternative = std::variant_alternative_t<Idx, UiOrderedStringMatcher>;

			static_assert (std::variant_size_v<UiOrderedStringMatcher> == std::variant_size_v<SVM::Pattern::Base>);

			static int ToUiIndex (const SVM::Pattern& matcher)
			{
				return Util::Visit (matcher, [] (const auto& val) { return UiOrderedStringMatcher { val }.index (); });
			}

			static QString ExtractPattern (const SVM::Pattern& matcher)
			{
				return Util::Visit (matcher,
						[] (const QRegularExpression& rx) { return rx.pattern (); },
						[] (const auto& val) { return val.Pattern_; });
			}

			static std::optional<SVM::Pattern> FromUiIndex (int index, const QString& pattern)
			{
				return [&]<size_t... Idx> (std::index_sequence<Idx...>)
				{
					// TODO C++26 template for
					std::optional<SVM::Pattern> result;
					static_cast<void> (((index == Idx && (result = UiOrderedAlternative<Idx> { pattern }, 0)) + ...));
					return result;
				} (std::make_index_sequence<std::variant_size_v<UiOrderedStringMatcher>> {});
			}
		};
	}

	IConfigWidget_ptr CreateMatcherConfigWidget (const AN::FieldData& field, const std::optional<AN::ValueMatcher>& matcher)
	{
		switch (field.Type_)
		{
		case QMetaType::Bool:
			return CreateTypedConfigWidget<BoolConfig> (matcher, field.Name_);
		case QMetaType::Int:
			return CreateTypedConfigWidget<IntConfig> (matcher);
		case QMetaType::QString:
		case QMetaType::QStringList:
			return CreateTypedConfigWidget<StringConfig> (matcher, field.AllowedValues_);
		case QMetaType::QUrl:
			return CreateTypedConfigWidget<StringConfig> (matcher);
		default:
			qWarning () << "unknown type" << field.Type_;
			return {};
		}
	}

	namespace
	{
		using SVM = AN::StringValueMatcher;

		QString GetTranslation (const AN::FieldData& field, const QString& label)
		{
			if (field.AllowedValues_.isEmpty ())
				return label;

			if (const auto pos = std::ranges::find (field.AllowedValues_, label, &AN::FieldData::AllowedValue::Id_);
				pos != field.AllowedValues_.end ())
				return pos->Name_;

			qWarning () << "cannot find" << label << "in" << field.ID_
					<< Util::Map (field.AllowedValues_, &AN::FieldData::AllowedValue::Id_);
			return label;
		}

		struct Descriptions
		{
			Q_DECLARE_TR_FUNCTIONS (LC::AdvancedNotifications::Descriptions)
		public:
			static QString ForBoolMatcher (const AN::BoolValueMatcher& bm)
			{
				return bm.Value_ ? tr ("is set") : tr ("is not set");
			}

			static QString ForIntMatcher (const AN::IntValueMatcher& im)
			{
				using enum AN::IntValueMatcher::Operation;

				QString op;
				if (im.Ops_ & OGreater)
					op += '>';
				if (im.Ops_ & OLess)
					op += '<';
				if (im.Ops_ & OEqual)
					op += '=';

				return op + ' ' + QString::number (im.Boundary_);
			}

			static QString ForStringMatcher (const AN::StringValueMatcher& value, const AN::FieldData& field)
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
							return msg.arg (GetTranslation (field, em.Pattern_));
						});
			}

			static QString ForStringListMatcher (const AN::StringValueMatcher& value, const AN::FieldData& field)
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
							return msg.arg (GetTranslation (field, em.Pattern_));
						});
			}
		};
	}

	QString GetMatcherDescription (const AN::FieldData& field, const AN::ValueMatcher& matcher)
	{
		return Util::Visit (matcher,
				[&] (const AN::BoolValueMatcher& bm) { return Descriptions::ForBoolMatcher (bm); },
				[&] (const AN::IntValueMatcher& im) { return Descriptions::ForIntMatcher (im); },
				[&] (const AN::StringValueMatcher& sm)
				{
					switch (field.Type_)
					{
					case QMetaType::QString:
					case QMetaType::QUrl:
						return Descriptions::ForStringMatcher (sm, field);
					case QMetaType::QStringList:
						return Descriptions::ForStringListMatcher (sm, field);
					default:
						qWarning () << "unknown type" << field.Type_ << "for field" << field.ID_;
						return QString {};
					}
				});
	}
}
