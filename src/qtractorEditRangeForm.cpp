// qtractorEditRangeForm.cpp
//
/****************************************************************************
   Copyright (C) 2005-2013, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorEditRangeForm.h"

#include "qtractorAbout.h"
#include "qtractorSession.h"

#include "qtractorOptions.h"

#include <QMessageBox>
#include <QPushButton>


//----------------------------------------------------------------------------
// qtractorEditRangeForm -- UI wrapper form.

// Constructor.
qtractorEditRangeForm::qtractorEditRangeForm (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Window modality (let plugin/tool windows rave around).
	QDialog::setWindowModality(Qt::ApplicationModal);

	// Initialize dirty control state.
	m_pTimeScale = NULL;

	// Initial selection range (empty).
	m_iSelectStart = 0;
	m_iSelectEnd   = 0;

    m_options = Clips | Automation;
    m_iUpdate = 0;

	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession) {
		m_pTimeScale = new qtractorTimeScale(*pSession->timeScale());
		m_ui.RangeStartSpinBox->setTimeScale(m_pTimeScale);
		m_ui.RangeEndSpinBox->setTimeScale(m_pTimeScale);
		// Set proper time scales display format...
		switch (m_pTimeScale->displayFormat()) {
		case qtractorTimeScale::BBT:
			m_ui.BbtRadioButton->setChecked(true);
			break;
		case qtractorTimeScale::Time:
			m_ui.TimeRadioButton->setChecked(true);
			break;
		case qtractorTimeScale::Frames:
		default:
			m_ui.FramesRadioButton->setChecked(true);
			break;
		}
		// Default selection is whole session...
		m_iSelectStart = pSession->sessionStart();
		m_iSelectEnd   = pSession->sessionEnd();
		// Populate range options...
		if (pSession->editHead() < pSession->editTail())
			m_ui.EditRangeRadioButton->setChecked(true);
		else
		if (pSession->isPunching())
			m_ui.PunchRangeRadioButton->setChecked(true);
		else
		if (pSession->isLooping())
			m_ui.LoopRangeRadioButton->setChecked(true);
		else
			m_ui.CustomRangeRadioButton->setChecked(true);
	}

    // Update options check-boxes.
    qtractorOptions *pOptions = qtractorOptions::getInstance();
    if (pOptions)
        m_options = pOptions->iEditRangeOptions;

    updateOptions();

	// Try to restore old window positioning.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.SelectionRangeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(rangeChanged()));
	QObject::connect(m_ui.LoopRangeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(rangeChanged()));
	QObject::connect(m_ui.PunchRangeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(rangeChanged()));
	QObject::connect(m_ui.EditRangeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(rangeChanged()));
	QObject::connect(m_ui.CustomRangeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(rangeChanged()));
	QObject::connect(m_ui.TimeRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(formatChanged()));
	QObject::connect(m_ui.BbtRadioButton,
		SIGNAL(toggled(bool)),
		SLOT(formatChanged()));
	QObject::connect(m_ui.RangeStartSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(valueChanged()));
	QObject::connect(m_ui.RangeEndSpinBox,
		SIGNAL(valueChanged(unsigned long)),
		SLOT(valueChanged()));
    QObject::connect(m_ui.ClipsCheckBox,
        SIGNAL(toggled(bool)),
        SLOT(optionsChanged()));
    QObject::connect(m_ui.AutomationCheckBox,
        SIGNAL(toggled(bool)),
        SLOT(optionsChanged()));
    QObject::connect(m_ui.TempoMapCheckBox,
        SIGNAL(toggled(bool)),
        SLOT(optionsChanged()));
    QObject::connect(m_ui.MarkersCheckBox,
        SIGNAL(toggled(bool)),
        SLOT(optionsChanged()));
    QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));

	// Done.
	stabilizeForm();
}


// Destructor.
qtractorEditRangeForm::~qtractorEditRangeForm (void)
{
	// Don't forget to get rid of local time-scale instance...
	if (m_pTimeScale)
		delete m_pTimeScale;
}


// Set the current initial selection range.
void qtractorEditRangeForm::setSelectionRange (
	unsigned long iSelectStart, unsigned long iSelectEnd )
{
	m_iSelectStart = iSelectStart;
	m_iSelectEnd = iSelectEnd;

	if (m_ui.CustomRangeRadioButton->isChecked()) {
		m_ui.RangeStartSpinBox->setValue(m_iSelectStart, false);
		m_ui.RangeEndSpinBox->setValue(m_iSelectEnd, false);
	}

	rangeChanged();
}


// Retrieve the current range, if the case arises.
unsigned long qtractorEditRangeForm::rangeStart (void) const
{
	return m_ui.RangeStartSpinBox->value();
}

unsigned long qtractorEditRangeForm::rangeEnd (void) const
{
	return m_ui.RangeEndSpinBox->value();
}


// Retrieve range option flags.
unsigned int qtractorEditRangeForm::rangeOptions (void) const
{
    return m_options;
}


// Option flags accessors.
void qtractorEditRangeForm::setOption ( Option option, bool bOn )
{
    if (bOn)
        m_options |=  (unsigned int) (option);
    else
        m_options &= ~(unsigned int) (option);
}

bool qtractorEditRangeForm::isOption ( Option option ) const
{
    return (m_options & (unsigned int) (option));
}


// Update options settings.
void qtractorEditRangeForm::updateOptions (void)
{
    ++m_iUpdate;
    m_ui.ClipsCheckBox->setChecked(m_options & Clips);
    m_ui.AutomationCheckBox->setChecked(m_options & Automation);
    m_ui.TempoMapCheckBox->setChecked(m_options & TempoMap);
    m_ui.MarkersCheckBox->setChecked(m_options & Markers);
    --m_iUpdate;
}


// Options changed.
void qtractorEditRangeForm::optionsChanged (void)
{
    if (m_iUpdate > 0)
        return;

    setOption(Clips, m_ui.ClipsCheckBox->isChecked());
    setOption(Automation, m_ui.AutomationCheckBox->isChecked());
    setOption(TempoMap, m_ui.TempoMapCheckBox->isChecked());
    setOption(Markers, m_ui.MarkersCheckBox->isChecked());

    stabilizeForm();
}


// Range settings have changed.
void qtractorEditRangeForm::rangeChanged (void)
{
	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return;

	if (m_ui.SelectionRangeRadioButton->isChecked()) {
		m_ui.RangeStartSpinBox->setValue(m_iSelectStart, false);
		m_ui.RangeEndSpinBox->setValue(m_iSelectEnd, false);
	}
	else
	if (m_ui.LoopRangeRadioButton->isChecked()) {
		m_ui.RangeStartSpinBox->setValue(pSession->loopStart(), false);
		m_ui.RangeEndSpinBox->setValue(pSession->loopEnd(), false);
	}
	else
	if (m_ui.PunchRangeRadioButton->isChecked()) {
		m_ui.RangeStartSpinBox->setValue(pSession->punchIn(), false);
		m_ui.RangeEndSpinBox->setValue(pSession->punchOut(), false);
	}
	else
	if (m_ui.EditRangeRadioButton->isChecked()) {
		m_ui.RangeStartSpinBox->setValue(pSession->editHead(), false);
		m_ui.RangeEndSpinBox->setValue(pSession->editTail(), false);
	}

	stabilizeForm();
}


// Range values have changed.
void qtractorEditRangeForm::valueChanged (void)
{
	m_ui.CustomRangeRadioButton->setChecked(true);

	stabilizeForm();
}


// Display format has changed.
void qtractorEditRangeForm::formatChanged (void)
{
	qtractorTimeScale::DisplayFormat displayFormat = qtractorTimeScale::Frames;

	if (m_ui.TimeRadioButton->isChecked())
		displayFormat = qtractorTimeScale::Time;
	else
	if (m_ui.BbtRadioButton->isChecked())
		displayFormat= qtractorTimeScale::BBT;

	if (m_pTimeScale) {
		// Set from local time-scale instance...
		m_pTimeScale->setDisplayFormat(displayFormat);
		m_ui.RangeStartSpinBox->updateDisplayFormat();
		m_ui.RangeEndSpinBox->updateDisplayFormat();
	}

	stabilizeForm();
}


// Stabilize current form state.
void qtractorEditRangeForm::stabilizeForm (void)
{
	qtractorSession *pSession = qtractorSession::getInstance();
	if (pSession == NULL)
		return;

	m_ui.SelectionRangeRadioButton->setEnabled(m_iSelectStart < m_iSelectEnd);
	m_ui.LoopRangeRadioButton->setEnabled(pSession->isLooping());
	m_ui.PunchRangeRadioButton->setEnabled(pSession->isPunching());
	m_ui.EditRangeRadioButton->setEnabled(
		pSession->editHead() < pSession->editTail());

    qtractorTimeScale *pTimeScale = pSession->timeScale();
    m_ui.TempoMapCheckBox->setEnabled(pTimeScale->nodes().count() > 1);
    m_ui.MarkersCheckBox->setEnabled(pTimeScale->markers().first() != NULL);

	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(
		m_ui.RangeStartSpinBox->value() < m_ui.RangeEndSpinBox->value());
}


// Dialog acceptance.
void qtractorEditRangeForm::accept (void)
{
    // Update options check-boxes.
    qtractorOptions *pOptions = qtractorOptions::getInstance();
    if (pOptions)
        pOptions->iEditRangeOptions = m_options;

    QDialog::accept();
}


// end of qtractorEditRangeForm.cpp
