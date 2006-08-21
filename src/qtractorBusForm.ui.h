// qtractorBusForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2005-2006, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "qtractorAbout.h"
#include "qtractorOptions.h"
#include "qtractorEngineCommand.h"
#include "qtractorAudioEngine.h"
#include "qtractorMidiEngine.h"

#include "qtractorMainForm.h"

#include <qmessagebox.h>
#include <qpopupmenu.h>


//----------------------------------------------------------------------
// class qtractorBusListItem -- Custom bus listview item.
//

class qtractorBusListItem : public QListViewItem
{
public:

	// Constructor.
	qtractorBusListItem(QListViewItem *pRootItem, qtractorBus *pBus)
		: QListViewItem(pRootItem, pBus->busName()), m_pBus(pBus)
	{
		switch (m_pBus->busType()) {
		case qtractorTrack::Audio:
			QListViewItem::setPixmap(0,
				QPixmap::fromMimeSource("trackAudio.png"));
			break;
		case qtractorTrack::Midi:
			QListViewItem::setPixmap(0,
				QPixmap::fromMimeSource("trackMidi.png"));
			break;
		default:
			break;
		}
	}

	// Bus accessors.
	qtractorBus *bus() const { return m_pBus; }

private:

	// Instance variables.
	qtractorBus *m_pBus;
};


// Kind of constructor.
void qtractorBusForm::init (void)
{
	// Initialize locals.
	m_pBus        = NULL;
	m_pAudioRoot  = NULL;
	m_pMidiRoot   = NULL;
	m_iDirtySetup = 0;
	m_iDirtyCount = 0;
	m_iDirtyTotal = 0;

	// Start with unsorted bus list...
	BusListView->setSorting(2);

	// (Re)initial contents.
	refreshBusses();

	// Try to restore normal window positioning.
	adjustSize();
}


// Kind of destructor.
void qtractorBusForm::destroy (void)
{
}


// Set current bus.
void qtractorBusForm::setBus ( qtractorBus *pBus )
{
	// Get the device view root item...
	QListViewItem *pRootItem = NULL;
	if (pBus) {
		switch (pBus->busType()) {
		case qtractorTrack::Audio:
			pRootItem = m_pAudioRoot;
			break;
		case qtractorTrack::Midi:
			pRootItem = m_pMidiRoot;
			break;
		default:
			break;
		}
	}
	// Is the root present?
	if (pRootItem == NULL) {
		stabilizeForm();
		return;
	}

	// For each child, test for identity...
	QListViewItem *pItem = pRootItem->firstChild();
	while (pItem) {
		// If identities match, select as current device item.
		qtractorBusListItem *pBusItem
			= static_cast<qtractorBusListItem *> (pItem);
		if (pBusItem && pBusItem->bus() == pBus) {
			BusListView->setSelected(pItem, true);
			break;
		}
		pItem = pItem->nextSibling();
	}
}


// Current bus accessor.
qtractorBus *qtractorBusForm::bus (void)
{
	return m_pBus;
}


// Current bus accessor.
bool qtractorBusForm::isDirty (void)
{
	return (m_iDirtyTotal > 0);
}


// Show current selected bus.
void qtractorBusForm::showBus ( qtractorBus *pBus )
{
	m_iDirtySetup++;

	// Settle current bus reference...
	m_pBus = pBus;

	// Show bus properties into view pane...
	if (pBus) {
		qtractorAudioBus *pAudioBus = NULL;
		switch (pBus->busType()) {
		case qtractorTrack::Audio:
			BusTitleTextLabel->setText(tr("Audio bus"));
			pAudioBus = static_cast<qtractorAudioBus *> (pBus);
			break;
		case qtractorTrack::Midi:
			BusTitleTextLabel->setText(tr("MIDI bus"));
			break;
		case qtractorTrack::None:
			BusTitleTextLabel->setText(tr("Bus"));
			break;
		}
		BusNameLineEdit->setText(pBus->busName());
		BusModeComboBox->setCurrentItem(int(pBus->busMode()) - 1);
		if (pAudioBus) {
			AudioChannelsSpinBox->setValue(pAudioBus->channels());
			AudioAutoConnectCheckBox->setChecked(pAudioBus->isAutoConnect());
		}
	}

	// Reset dirty flag...
	m_iDirtyCount = 0;	
	m_iDirtySetup--;
	
	// Done.
	stabilizeForm();
}


// Refresh all busses list and views.
void qtractorBusForm::refreshBusses (void)
{
	//
	// (Re)Load complete bus listing ...
	//
	m_pAudioRoot = NULL;
	m_pMidiRoot  = NULL;
	BusListView->clear();

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return;
	qtractorSession *pSession = pMainForm->session();
	if (pSession == NULL)
		return;

	// MIDI busses...
	qtractorMidiEngine *pMidiEngine = pSession->midiEngine();
	if (pMidiEngine) {
		m_pMidiRoot = new QListViewItem(BusListView, ' ' + tr("MIDI"));
		m_pMidiRoot->setSelectable(false);
		for (qtractorBus *pBus = pMidiEngine->busses().last();
				pBus; pBus = pBus->prev())
			new qtractorBusListItem(m_pMidiRoot, pBus);
		m_pMidiRoot->setOpen(true);
	}

	// Audio busses...
	qtractorAudioEngine *pAudioEngine = pSession->audioEngine();
	if (pAudioEngine) {
		m_pAudioRoot = new QListViewItem(BusListView, ' ' + tr("Audio"));
		m_pAudioRoot->setSelectable(false);
		for (qtractorBus *pBus = pAudioEngine->busses().last();
				pBus; pBus = pBus->prev())
			new qtractorBusListItem(m_pAudioRoot, pBus);
		m_pAudioRoot->setOpen(true);
	}

	// Reselect current bus, if any.
	setBus(m_pBus);
}


// Bus selection slot.
void qtractorBusForm::selectBus (void)
{
	// Get current selected item, must not be a root one...
	QListViewItem *pItem = BusListView->selectedItem();
	if (pItem == NULL)
		return;
	if (pItem->parent() == NULL)
		return;

	// Just make it in current view...
	qtractorBusListItem *pBusItem
		= static_cast<qtractorBusListItem *> (pItem);
	if (pBusItem == NULL)
		return;

	// Check if we need an update?...
	if (m_pBus && pBusItem->bus() != m_pBus && m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			tr("Apply"), tr("Discard"), tr("Cancel"))) {
		case 0:     // Apply
			updateBus();
			break;
		case 1:     // Discard
			break;;
		default:    // Cancel.
			return;
		}
	}

	// Get new one into view...
	showBus(pBusItem->bus());
}


// Check whether the current view is elligible as a new bus.
bool qtractorBusForm::canCreateBus (void)
{
	if (m_iDirtyCount == 0)
		return false;
	if (m_pBus == NULL)
		return false;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return false;
	qtractorSession *pSession = pMainForm->session();
	if (pSession == NULL)
		return false;

	const QString sBusName = BusNameLineEdit->text().stripWhiteSpace();
	if (sBusName.isEmpty())
		return false;

	// Get the device view root item...
	qtractorEngine *pEngine = NULL;
	switch (m_pBus->busType()) {
	case qtractorTrack::Audio:
		pEngine = pSession->audioEngine();
		break;
	case qtractorTrack::Midi:
		pEngine = pSession->midiEngine();
		break;
	default:
		break;
	}
	// Is it still valid?
	if (pEngine == NULL)
		return false;

	// Is there one already?
	return (pEngine->findBus(sBusName) == NULL);
}


// Check whether the current view is elligible for update.
bool qtractorBusForm::canUpdateBus (void)
{
	if (m_iDirtyCount == 0)
		return false;
	if (m_pBus == NULL)
		return false;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return false;
	qtractorSession *pSession = pMainForm->session();
	if (pSession == NULL)
		return false;

	const QString sBusName = BusNameLineEdit->text().stripWhiteSpace();
	return (!sBusName.isEmpty());
}


// Check whether the current view is elligible for deletion.
bool qtractorBusForm::canDeleteBus (void)
{
	if (m_pBus == NULL)
		return false;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return false;
	qtractorSession *pSession = pMainForm->session();
	if (pSession == NULL)
		return false;

	// The very first bus is never deletable...
	return (m_pBus->prev() != NULL);
}


// Create a new bus from current view.
void qtractorBusForm::createBus (void)
{
	if (m_pBus == NULL)
		return;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return;
	
	const QString sBusName = BusNameLineEdit->text().stripWhiteSpace();
	if (sBusName.isEmpty())
		return;

	qtractorBus::BusMode busMode = qtractorBus::None;
	switch (BusModeComboBox->currentItem()) {
	case 0:
		busMode = qtractorBus::Input;
		break;
	case 1:
		busMode = qtractorBus::Output;
		break;
	case 2:
		busMode = qtractorBus::Duplex;
		break;
	}

	// Make it as an unduable command...
	qtractorCreateBusCommand *pCreateBusCommand
		= new qtractorCreateBusCommand(pMainForm);

	// Set all creational properties...
	qtractorTrack::TrackType busType = m_pBus->busType();
	pCreateBusCommand->setBusType(busType);
	pCreateBusCommand->setBusName(sBusName);
	pCreateBusCommand->setBusMode(busMode);	
	// Specialties for Audio bussess...
	if (busType == qtractorTrack::Audio)  {
		pCreateBusCommand->setChannels(AudioChannelsSpinBox->value());
		pCreateBusCommand->setAutoConnect(
			AudioAutoConnectCheckBox->isChecked());
	}

	// Execute and refresh form...
	if (pMainForm->commands()->exec(pCreateBusCommand)) {
		m_iDirtyTotal++;
		refreshBusses();
	}
}


// Update current bus in view.
void qtractorBusForm::updateBus (void)
{
	if (m_pBus == NULL)
		return;

	if (m_pBus == NULL)
		return;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return;
	
	const QString sBusName = BusNameLineEdit->text().stripWhiteSpace();
	if (sBusName.isEmpty())
		return;

	qtractorBus::BusMode busMode = qtractorBus::None;
	switch (BusModeComboBox->currentItem()) {
	case 0:
		busMode = qtractorBus::Input;
		break;
	case 1:
		busMode = qtractorBus::Output;
		break;
	case 2:
		busMode = qtractorBus::Duplex;
		break;
	}

	// Make it as an unduable command...
	qtractorUpdateBusCommand *pUpdateBusCommand
		= new qtractorUpdateBusCommand(pMainForm, m_pBus);

	// Set all updated properties...
	qtractorTrack::TrackType busType = m_pBus->busType();
	pUpdateBusCommand->setBusType(busType);
	pUpdateBusCommand->setBusName(sBusName);
	pUpdateBusCommand->setBusMode(busMode);	
	// Specialties for Audio bussess...
	if (busType == qtractorTrack::Audio)  {
		pUpdateBusCommand->setChannels(AudioChannelsSpinBox->value());
		pUpdateBusCommand->setAutoConnect(
			AudioAutoConnectCheckBox->isChecked());
	}

	// Execute and refresh form...
	if (pMainForm->commands()->exec(pUpdateBusCommand)) {
		m_iDirtyTotal++;
		refreshBusses();
	}
}


// Delete current bus in view.
void qtractorBusForm::deleteBus (void)
{
	if (m_pBus == NULL)
		return;

	qtractorMainForm *pMainForm = qtractorMainForm::getInstance();
	if (pMainForm == NULL)
		return;

	// Prompt user if he/she's sure about this...
	qtractorOptions *pOptions = pMainForm->options();
	if (pOptions && pOptions->bConfirmRemove) {
		// Get some textual type...
		QString sBusType;
		switch (m_pBus->busType()) {
		case qtractorTrack::Audio:
			sBusType = tr("Audio");
			break;
		case qtractorTrack::Midi:
			sBusType = tr("MIDI");
			break;
		default:
			break;
		}
		// Show the warning...
		if (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("About to remove bus:\n\n"
			"%1 (%2)\n\n"
			"Are you sure?")
			.arg(m_pBus->busName())
			.arg(sBusType),
			tr("OK"), tr("Cancel")) > 0)
			return;
	}

	// Make it as an unduable command...
	qtractorDeleteBusCommand *pDeleteBusCommand
		= new qtractorDeleteBusCommand(pMainForm, m_pBus);

	// Execute and refresh form...
	if (pMainForm->commands()->exec(pDeleteBusCommand)) {
		m_iDirtyTotal++;
		refreshBusses();
	}
}


// Make changes due.
void qtractorBusForm::changed (void)
{
	if (m_iDirtySetup > 0)
		return;

	m_iDirtyCount++;
	stabilizeForm();
}


// Reject settings (Close button slot).
void qtractorBusForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("Some settings have been changed.\n\n"
			"Do you want to discard the changes?"),
			tr("Discard"), tr("Cancel"))) {
		case 0:     // Discard
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Stabilize current form state.
void qtractorBusForm::stabilizeForm (void)
{
	if (m_pBus) {
		CommonBusGroup->setEnabled(true);
		AudioBusGroup->setEnabled(m_pBus->busType() == qtractorTrack::Audio);
	} else {
		CommonBusGroup->setEnabled(false);
		AudioBusGroup->setEnabled(false);
	}

	RefreshPushButton->setEnabled(m_iDirtyCount > 0);
	CreatePushButton->setEnabled(canCreateBus());
	UpdatePushButton->setEnabled(canUpdateBus());
	DeletePushButton->setEnabled(canDeleteBus());
}


// Bus list view context menu handler.
void qtractorBusForm::contextMenu ( QListViewItem *, const QPoint& pos, int )
{
	int iItemID;

	// Build the device context menu...
	QPopupMenu* pContextMenu = new QPopupMenu(this);

	iItemID = pContextMenu->insertItem(
		QIconSet(QPixmap::fromMimeSource("formCreate.png")),
		tr("&Create"), this, SLOT(createBus()));
	pContextMenu->setItemEnabled(iItemID, canCreateBus());
	iItemID = pContextMenu->insertItem(
		QIconSet(QPixmap::fromMimeSource("formAccept.png")),
		tr("&Update"), this, SLOT(updateBus()));
	pContextMenu->setItemEnabled(iItemID, canUpdateBus());
	iItemID = pContextMenu->insertItem(
		QIconSet(QPixmap::fromMimeSource("formRemove.png")),
		tr("&Delete"), this, SLOT(deleteBus()));
	pContextMenu->setItemEnabled(iItemID, canDeleteBus());
	pContextMenu->insertSeparator();
	iItemID = pContextMenu->insertItem(
		QIconSet(QPixmap::fromMimeSource("formRefresh.png")),
		tr("&Refresh"), this, SLOT(refreshBusses()));
	pContextMenu->setItemEnabled(iItemID, m_iDirtyCount > 0);

	pContextMenu->exec(pos);

	delete pContextMenu;
}


// end of qtractorBusForm.ui.h
