#include <QtGui/QtGui>
#include <QtDebug>
#include <QMetaType>
#include "converters.h"
#include "datatypes.h"
#include "settingsiteminfo.h"
#include "settingsdialog.h"
#include "filepickerwidget.h"
#include "extendedspinbox.h"
#include "collectorwidget.h"

void Converter::SetType (unsigned int type)
{
	Type_ = type;
}

Converter::Converter (QObject *parent)
: QObject (parent)
{
}

Converter::~Converter ()
{
	QList<QWidget*> keys = Widget2Object_.keys ();
	for (int i = 0; i < keys.size (); ++i)
	{
		delete keys [i];
		keys [i] = 0;
	}
}

bool Converter::MakeLabel () const
{
	return true;
}

unsigned int Converter::GetType () const
{
	return Type_;
} 

void Converter::UpdateSettings ()
{
	for (int i = 0; i < ModifiedProperties_.size (); ++i)
		Widget2Object_ [ModifiedProperties_ [i]]->setProperty (Widget2Property_ [ModifiedProperties_ [i]].toAscii ().data (), ReadSetting (ModifiedProperties_ [i]));
}

void Converter::updateSetting ()
{
	if (!sender ())
		return;
	QWidget *w = qobject_cast<QWidget*> (sender ());
	if (!w)
	{
		qDebug () << Q_FUNC_INFO << "unsuccessful qobject_cast to qwidget.";
		return;
	}

	for (int i = 0; i < ModifiedProperties_.size (); ++i)
		if (ModifiedProperties_ [i]->winId () == w->winId ())
			return;
	ModifiedProperties_.append (w);
}

QStringConverter::QStringConverter (QObject *parent)
: Converter (parent)
{
	SetType (QVariant::String);
}

QStringConverter::~QStringConverter ()
{
}

QWidget* QStringConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	QWidget *result;

	if (sii.BrowseButton_)
	{
		result = new FilePickerWidget;
		qobject_cast<FilePickerWidget*> (result)->setText (value.toString ());
	}
	else
	{
		result = new QLineEdit;
		qobject_cast<QLineEdit*> (result)->setText (value.toString ());
	}

	Widget2Property_ [result] = propName;
	Widget2Object_ [result] = owner;
	connect (result, SIGNAL (textChanged (const QString&)), this, SLOT (updateSetting ()));
	return result;
}

QVariant QStringConverter::ReadSetting (QWidget *w) const
{
	QLineEdit *le = qobject_cast<QLineEdit*> (w);
	FilePickerWidget *fpw = qobject_cast<FilePickerWidget*> (w);
	if (fpw)
		return fpw->text ();
	else if (le)
		return le->text ();
	else
		return QVariant ();
}

IntConverter::IntConverter (QObject *parent)
: Converter (parent)
{
	SetType (QVariant::Int);
}

IntConverter::~IntConverter ()
{
}

QWidget* IntConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	QSpinBox *result = new QSpinBox;
	result->setMinimum (sii.IntRange_.first);
	result->setMaximum (sii.IntRange_.second);
	result->setSuffix (sii.SpinboxSuffix_);
	result->setSingleStep (sii.SpinboxStep_);
	result->setValue (value.toInt ());
	Widget2Property_ [result] = propName;
	Widget2Object_ [result] = owner;
	connect (result, SIGNAL (valueChanged (int)), this, SLOT (updateSetting ()));
	return result;
}

QVariant IntConverter::ReadSetting (QWidget *w) const
{
	QSpinBox *sb = qobject_cast<QSpinBox*> (w);
	if (!sb)
		return QVariant ();
	return sb->value ();
}

UIntConverter::UIntConverter (QObject *parent)
: Converter (parent)
{
	SetType (QVariant::UInt);
}

UIntConverter::~UIntConverter ()
{
}

QWidget* UIntConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	QSpinBox *result = new QSpinBox;
	result->setMinimum (sii.UIntRange_.first);
	result->setMaximum (sii.UIntRange_.second);
	result->setSuffix (sii.SpinboxSuffix_);
	result->setSingleStep (sii.SpinboxStep_);
	result->setValue (value.toUInt ());
	Widget2Property_ [result] = propName;
	Widget2Object_ [result] = owner;
	connect (result, SIGNAL (valueChanged (int)), this, SLOT (updateSetting ()));
	return result;
}

QVariant UIntConverter::ReadSetting (QWidget *w) const
{
	QSpinBox *sb = qobject_cast<QSpinBox*> (w);
	if (!sb)
		return QVariant ();
	return sb->value ();
}

BoolConverter::BoolConverter (QObject *parent)
: Converter (parent)
{
	SetType (QVariant::Bool);
}

BoolConverter::~BoolConverter ()
{
}

QWidget* BoolConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	if (sii.GroupBoxer_)
	{
		QGroupBox *result = new QGroupBox (sii.Label_);
		Widget2Property_ [result] = propName;
		Widget2Object_ [result] = owner;
		connect (result, SIGNAL (toggled (bool)), this, SLOT (updateSetting ()));
		result->setCheckable (true);
		result->setChecked (value.toBool () ? Qt::Checked : Qt::Unchecked);
		result->setLayout (new QVBoxLayout);
		for (int i = 0; i < sii.SubItems_.size (); ++i)
			qobject_cast<SettingsDialog*> (parent ())->SetParentWidgetForProperty (sii.SubItems_.at (i), result);
		return result;
	}
	else
	{
		QCheckBox *result = new QCheckBox (sii.Label_);
		Widget2Property_ [result] = propName;
		Widget2Object_ [result] = owner;
		connect (result, SIGNAL (stateChanged (int)), this, SLOT (updateSetting ()));
		result->setCheckState (value.toBool () ? Qt::Checked : Qt::Unchecked);
		return result;
	}
}

QVariant BoolConverter::ReadSetting (QWidget *w) const
{
	QCheckBox *cb = qobject_cast<QCheckBox*> (w);
	QGroupBox *gb = qobject_cast<QGroupBox*> (w);
	if (cb)
		return cb->checkState () == Qt::Checked ? true : false;
	else if (gb)
		return gb->isChecked ();
	else
		return QVariant ();
}

bool BoolConverter::MakeLabel () const
{
	return false;
}

QStringListConverter::QStringListConverter (QObject *parent)
: Converter (parent)
{
	SetType (QVariant::StringList);
}

QStringListConverter::~QStringListConverter ()
{
}

QWidget* QStringListConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	QTextEdit *result = new QTextEdit (value.toStringList ().join (""));
	result->setAcceptRichText (false);
	result->setLineWrapMode (QTextEdit::NoWrap);
	Widget2Property_ [result] = propName;
	Widget2Object_ [result] = owner;
	connect (result, SIGNAL (textChanged ()), this, SLOT (updateSetting ()));
	result->resize (250, 100);
	return result;
}

bool QStringListConverter::MakeLabel () const
{
	return true;
}

QVariant QStringListConverter::ReadSetting (QWidget *w) const
{
	QTextEdit *result = qobject_cast<QTextEdit*> (w);
	if (!result)
		return QVariant ();

	QString text = result->toPlainText ();
	if (text.isEmpty ())
		return QVariant ();

	QStringList list = text.split ('\n', QString::SkipEmptyParts);
	// Maybe we're on Mac or somewhere else where end of string is \r without \n
	if (list.isEmpty ())
		list = text.split ('\r', QString::SkipEmptyParts);

	for (int i = 0; i < list.size (); ++i)
		list [i] = list [i].trimmed ();

	return QVariant (list);
}

PairedStringListConverter::PairedStringListConverter (QObject *parent)
: Converter (parent)
{
	SetType (qRegisterMetaType<PairedStringList> ("PairedStringList"));
}

PairedStringListConverter::~PairedStringListConverter ()
{
}

QWidget* PairedStringListConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	if (!value.canConvert<PairedStringList> ())
		return 0;

	PairedStringList strings = value.value<PairedStringList> ();
	QComboBox *result = new QComboBox;
	Widget2Property_ [result] = propName;
	Widget2Object_ [result] = owner;
	result->addItems (strings.first);
	result->setCurrentIndex (strings.second);
	result->setEditable (false);
	connect (result, SIGNAL (currentIndexChanged (int)), this, SLOT (updateSetting ()));

	return result;
}

bool PairedStringListConverter::MakeLabel () const
{
	return true;
}

QVariant PairedStringListConverter::ReadSetting (QWidget *w) const
{
	QComboBox *box = qobject_cast<QComboBox*> (w);
	if (!box)
		return QVariant ();

	PairedStringList result;
	result.second = box->currentIndex ();
	for (int i = 0; i < box->count (); ++i)
		result.first << box->itemText (i);

	return QVariant::fromValue<PairedStringList> (result);
}

IntRangeConverter::IntRangeConverter (QObject *parent)
: Converter (parent)
{
	SetType (qRegisterMetaType<IntRange> ("IntRange"));
}

IntRangeConverter::~IntRangeConverter ()
{
}

QWidget* IntRangeConverter::Convert (const QVariant& value, const SettingsItemInfo& sii, const QString& propName, QObject *owner)
{
	if (!value.canConvert<IntRange> ())
		return 0;

	IntRange range = value.value<IntRange> ();
	CollectorWidget *result = new CollectorWidget;
	QHBoxLayout *lay = new QHBoxLayout (result);
	result->setLayout (lay);

	ExtendedSpinbox *low = new ExtendedSpinbox (result),
					*high = new ExtendedSpinbox (result);
	low->setObjectName ("low");
	high->setObjectName ("high");
	low->setRange (sii.IntRange_.first, range.second);
	high->setRange (range.first, sii.IntRange_.second);
	low->setSingleStep (sii.SpinboxStep_);
	high->setSingleStep (sii.SpinboxStep_);
	low->setSuffix (sii.SpinboxSuffix_);
	high->setSuffix (sii.SpinboxSuffix_);
	low->SetMaximumShift (-1);
	high->SetMinimumShift (1);
	low->setValue (range.first);
	high->setValue (range.second);

	connect (low, SIGNAL (valueChanged (int)), high, SLOT (changeMinimum (int)));
	connect (high, SIGNAL (valueChanged (int)), low, SLOT (changeMaximum (int)));
	connect (low, SIGNAL (valueChanged (int)), result, SIGNAL (collected ()));
	connect (high, SIGNAL (valueChanged (int)), result, SIGNAL (collected ()));
	connect (result, SIGNAL (collected ()), this, SLOT (updateSetting ()));

	lay->addWidget (low);
	lay->addWidget (high);
	lay->addStretch ();

	Widget2Property_ [result] = propName;
	Widget2Object_ [result] = owner;
	return result;
}

QVariant IntRangeConverter::ReadSetting (QWidget *w) const
{
	QList<ExtendedSpinbox*> lows = w->findChildren<ExtendedSpinbox*> ("low"),
							highs = w->findChildren<ExtendedSpinbox*> ("high");
	if (lows.size () != 1 || highs.size () != 1)
		return QVariant ();

	return QVariant::fromValue<IntRange> (qMakePair<int, int> (lows.at (0)->value (), highs.at (0)->value ()));
}

bool IntRangeConverter::MakeLabel () const
{
	return true;
}


