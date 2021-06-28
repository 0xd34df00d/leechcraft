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
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
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
			return result;
		}
	}

	TypedMatcherBase_ptr TypedMatcherBase::Create (QVariant::Type type, const ANFieldData& fieldData)
	{
		switch (type)
		{
		case QVariant::Bool:
			return std::make_shared<BoolMatcher> (fieldData.Name_);
		case QVariant::Int:
			return std::make_shared<IntMatcher> ();
		case QVariant::String:
			return std::make_shared<StringMatcher> (ToTList<QString> (fieldData.AllowedValues_));
		case QVariant::StringList:
			return std::make_shared<StringListMatcher> (ToTList<QString> (fieldData.AllowedValues_));
		case QVariant::Url:
			return std::make_shared<UrlMatcher> ();
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown type"
					<< type;
			return {};
		}
	}

	StringLikeMatcher::StringLikeMatcher (const QStringList& variants)
	: Allowed_ (variants)
	{
	}

	QVariantMap StringLikeMatcher::Save () const
	{
		return
		{
			{ "Rx", Value_.Rx_ },
			{ "Cont", Value_.Contains_ }
		};
	}

	void StringLikeMatcher::Load (const QVariantMap& map)
	{
		Value_.Rx_ = map ["Rx"].toRegExp ();
		Value_.Contains_ = map ["Cont"].toBool ();
	}

	namespace
	{
		template<typename T>
		void SetValueFromVariant (T& value, const ANFieldValue& variant)
		{
			Util::Visit (variant,
					[&value] (const T& val) { value = val; },
					[] (const auto&) {});
		}
	}

	void StringLikeMatcher::SetValue (const ANFieldValue& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void StringLikeMatcher::SetValue (const QVariant& variant)
	{
		Value_.Rx_ = QRegExp { variant.toString (), Qt::CaseSensitive, QRegExp::FixedString };
		Value_.Contains_ = true;
	}

	ANFieldValue StringLikeMatcher::GetValue () const
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

	void StringLikeMatcher::SyncToWidget ()
	{
		if (!CW_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Value_.Contains_ = Ui_->ContainsBox_->currentIndex () == 0;
		if (Allowed_.isEmpty ())
		{
			QRegExp::PatternSyntax pattern = QRegExp::FixedString;
			switch (Ui_->RegexType_->currentIndex ())
			{
			case 0:
				break;
			case 1:
				pattern = QRegExp::Wildcard;
				break;
			case 2:
				pattern = QRegExp::RegExp;
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown regexp type"
						<< Ui_->RegexType_->currentIndex ();
				break;
			}

			Value_.Rx_ = QRegExp (Ui_->RegexpEditor_->text (),
					Qt::CaseInsensitive, pattern);
		}
		else
			Value_.Rx_ = QRegExp (Ui_->VariantsBox_->currentText (),
					Qt::CaseSensitive, QRegExp::FixedString);
	}

	void StringLikeMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Ui_->ContainsBox_->setCurrentIndex (!Value_.Contains_);
		if (Allowed_.isEmpty ())
		{
			Ui_->RegexpEditor_->setText (Value_.Rx_.pattern ());

			switch (Value_.Rx_.patternSyntax ())
			{
			case QRegExp::FixedString:
				Ui_->RegexType_->setCurrentIndex (0);
				break;
			case QRegExp::Wildcard:
			case QRegExp::WildcardUnix:
				Ui_->RegexType_->setCurrentIndex (1);
				break;
			case QRegExp::RegExp:
			case QRegExp::RegExp2:
				Ui_->RegexType_->setCurrentIndex (2);
				break;
			case QRegExp::W3CXmlSchema11:
				qWarning () << Q_FUNC_INFO
						<< "unexpected regexp type"
						<< Value_.Rx_.patternSyntax ();
				break;
			}
		}
		else
		{
			const auto& pattern = Value_.Rx_.pattern ();
			const auto idx = Ui_->VariantsBox_->findText (pattern);
			if (idx == -1)
				qWarning () << Q_FUNC_INFO
						<< "cannot find pattern"
						<< pattern
						<< "in"
						<< Allowed_;
			else
				Ui_->VariantsBox_->setCurrentIndex (idx);
		}
	}

	bool StringMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<QString> ())
			return false;

		bool res = Value_.Rx_.indexIn (var.toString ()) != -1;
		if (!Value_.Contains_)
			res = !res;
		return res;
	}

	QString StringMatcher::GetHRDescription () const
	{
		const QString& p = Value_.Rx_.pattern ();
		return Value_.Contains_ ?
				QObject::tr ("contains pattern `%1`").arg (p) :
				QObject::tr ("doesn't contain pattern `%1`").arg (p);
	}

	StringListMatcher::StringListMatcher (const QStringList& list)
	: StringLikeMatcher (list)
	{
	}

	bool StringListMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<QStringList> ())
			return false;

		bool res = var.toStringList ().indexOf (Value_.Rx_) == -1;
		if (!Value_.Contains_)
			res = !res;
		return res;
	}

	QString StringListMatcher::GetHRDescription () const
	{
		const QString& p = Value_.Rx_.pattern ();
		return Value_.Contains_ ?
				QObject::tr ("contains element matching %1").arg (p) :
				QObject::tr ("doesn't contain element matching %1").arg (p);
	}

	bool UrlMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<QUrl> ())
			return false;

		const auto& url = var.toUrl ();
		const auto contains = url.toString ().indexOf (Value_.Rx_) != -1 ||
				QString::fromUtf8 (url.toEncoded ()).indexOf (Value_.Rx_) != -1;
		return contains == Value_.Contains_;
	}

	QString UrlMatcher::GetHRDescription () const
	{
		const QString& p = Value_.Rx_.pattern ();
		return Value_.Contains_ ?
				QObject::tr ("matches URL or pattern `%1`").arg (p) :
				QObject::tr ("doesn't match URL or pattern `%1`").arg (p);
	}

	BoolMatcher::BoolMatcher (const QString& fieldName)
	: FieldName_ { fieldName }
	{
	}

	QVariantMap BoolMatcher::Save () const
	{
		return { { "IsSet", Value_.IsSet_ } };
	}

	void BoolMatcher::Load (const QVariantMap& map)
	{
		Value_.IsSet_ = map.value ("IsSet").toBool ();
	}

	void BoolMatcher::SetValue (const ANFieldValue& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void BoolMatcher::SetValue (const QVariant& variant)
	{
		Value_.IsSet_ = variant.toBool ();
	}

	ANFieldValue BoolMatcher::GetValue () const
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
		Ops2pos_ [ANIntFieldValue::OGreater] = 0;
		Ops2pos_ [ANIntFieldValue::OEqual | ANIntFieldValue::OGreater] = 1;
		Ops2pos_ [ANIntFieldValue::OEqual] = 2;
		Ops2pos_ [ANIntFieldValue::OEqual | ANIntFieldValue::OLess] = 3;
		Ops2pos_ [ANIntFieldValue::OLess] = 4;
	}

	QVariantMap IntMatcher::Save () const
	{
		QVariantMap result;
		result ["Bd"] = Value_.Boundary_;
		result ["Ops"] = static_cast<quint16> (Value_.Ops_);
		return result;
	}

	void IntMatcher::Load (const QVariantMap& map)
	{
		Value_.Boundary_ = map ["Bd"].toInt ();
		Value_.Ops_ = static_cast<ANIntFieldValue::Operations> (map ["Ops"].value<quint16> ());
	}

	void IntMatcher::SetValue (const ANFieldValue& variant)
	{
		SetValueFromVariant (Value_, variant);
	}

	void IntMatcher::SetValue (const QVariant& variant)
	{
		Value_.Boundary_ = variant.toInt ();
		Value_.Ops_ = ANIntFieldValue::OEqual;
	}

	ANFieldValue IntMatcher::GetValue () const
	{
		return Value_;
	}

	bool IntMatcher::Match (const QVariant& var) const
	{
		if (!var.canConvert<int> ())
			return false;

		const int val = var.toInt ();

		if ((Value_.Ops_ & ANIntFieldValue::OEqual) && val == Value_.Boundary_)
			return true;
		if ((Value_.Ops_ & ANIntFieldValue::OGreater) && val > Value_.Boundary_)
			return true;
		if ((Value_.Ops_ & ANIntFieldValue::OLess) && val < Value_.Boundary_)
			return true;

		return false;
	}

	QString IntMatcher::GetHRDescription () const
	{
		if (Value_.Ops_ == ANIntFieldValue::OEqual)
			return QObject::tr ("equals to %1").arg (Value_.Boundary_);

		QString op;
		if ((Value_.Ops_ & ANIntFieldValue::OGreater))
			op += ">"_ql;
		if ((Value_.Ops_ & ANIntFieldValue::OLess))
			op += "<"_ql;
		if ((Value_.Ops_ & ANIntFieldValue::OEqual))
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
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Value_.Boundary_  = Ui_->Boundary_->value ();
		Value_.Ops_ = Ops2pos_.key (Ui_->OpType_->currentIndex ());
	}

	void IntMatcher::SyncWidgetTo ()
	{
		if (!CW_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null CW";
			return;
		}

		Ui_->Boundary_->setValue (Value_.Boundary_);
		Ui_->OpType_->setCurrentIndex (Ops2pos_ [Value_.Ops_]);
	}
}
