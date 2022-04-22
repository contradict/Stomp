/*
# Copyright 2021 Ball Aerospace & Technologies Corp.
# All Rights Reserved.
#
# This program is free software; you can modify and/or redistribute it
# under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation; version 3 with
# attribution addendums as found in the LICENSE.txt
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# This program may also be used under the terms of a commercial or
# enterprise edition license of COSMOS if purchased from the
# copyright holder
*/

import { format } from 'date-fns'

describe('DataViewer', () => {
  beforeEach(() => {
    cy.task('clearDownloads')
    cy.visit('/tools/dataviewer')
    cy.hideNav()
    cy.wait(1000)
  })

  it('adds a raw packet to a new tab', () => {
    cy.hideNav()
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]')
      .should('be.visible')
      .click({ force: true })
      .wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.wait(1000)
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.wait(1000) // wait for the first packet to come in
    cy.get('[data-test=dump-component-text-area]').should('not.have.value', '')
  })

  it('adds a decom packet to a new tab', () => {
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]').click({ force: true }).wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.get('[data-test=new-packet-decom-radio]').check({ force: true })
    cy.get('[data-test=add-packet-value-type]').should('be.visible')
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.wait(1000) // wait for the first packet to come in
    // add another packet to the existing connection
    cy.get('[data-test=new-packet]').click({ force: true })
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.wait(1000)
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    cy.get('[data-test=dump-component-text-area]').should('not.have.value', '')
  })

  it('renames a tab', () => {
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=tab]').rightclick({ force: true }).wait(1000)
    cy.get('[data-test=context-menu-rename]')
      .find('div')
      .click({ force: true })
      .wait(1000)
    cy.get('[data-test=rename-tab-input]')
      .clear({ force: true })
      .type('Testing tab name')
    cy.get('[data-test=rename]').click({ force: true }).wait(1000)
    cy.get('.v-tab').should('contain', 'Testing tab name')
    cy.get('[data-test=tab]').rightclick({ force: true }).wait(1000)
    cy.get('[data-test=context-menu-rename]')
      .find('div')
      .click({ force: true })
      .wait(1000)
    cy.get('[data-test=rename-tab-input]')
      .clear({ force: true })
      .type('Cancel this')
    cy.get('[data-test=cancel-rename]').click({ force: true }).wait(1000)
    cy.get('.v-tab').should('contain', 'Testing tab name')
  })

  it('deletes a component and tab', () => {
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]').click({ force: true }).wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.wait(1000)
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    cy.get('.v-window-item > .v-card > .v-card__title').should(
      'contain',
      'INST ADCS'
    )
    cy.get('[data-test=delete-packet]').click({ force: true }).wait(1000)
    cy.get('.v-window-item > .v-card > .v-card__title').should(
      'contain',
      'This tab is empty'
    )
    cy.get('[data-test=tab]').rightclick({ force: true }).wait(1000)
    cy.get('[data-test=context-menu-delete]')
      .find('div')
      .click({ force: true })
      .wait(1000)
    cy.get(':nth-child(4) > .v-card > .v-card__title').should(
      'contain',
      "You're not viewing any packets"
    )
  })

  it('controls playback', () => {
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]').click({ force: true }).wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.wait(1000)
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.wait(1000) // wait for the first packet to come in

    cy.get('[data-test=dump-component-play-pause]').click({ force: true })
    cy.get('[data-test=dump-component-text-area]')
      .invoke('val')
      .then((val) => {
        // ensure it paused
        cy.wait(500)
        cy.get('[data-test=dump-component-text-area]').should('have.value', val)

        // check stepper buttons
        cy.get(
          '.container > :nth-child(2) > .col > .v-input > .v-input__prepend-outer > .v-input__icon > .v-icon'
        )
          .click({ force: true })
          .wait(1000) // step back
        cy.get('[data-test=dump-component-text-area]').should(
          'not.have.value',
          val
        )
        cy.get('.v-input__append-outer > .v-input__icon > .v-icon')
          .click({
            force: true,
          })
          .wait(1000) // step forward
        cy.get('[data-test=dump-component-text-area]').should('have.value', val)

        // ensure it resumes
        cy.get('[data-test=dump-component-play-pause]')
          .click({ force: true })
          .wait(1000)
        cy.get('[data-test=dump-component-text-area]').should(
          'not.have.value',
          val
        )

        cy.get('[data-test=stop-button]').click({ force: true }).wait(1000)
        cy.wait(200) // give it time to unsubscribe and stop receiving packets
        return cy.get('[data-test=dump-component-text-area]').invoke('val')
      })
      .then((val) => {
        // ensure it stopped
        cy.wait(500)
        cy.get('[data-test=dump-component-text-area]').should('have.value', val)
      })
  })

  it('changes display settings', () => {
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]').click({ force: true }).wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.wait(1000)
    cy.get('[data-test=add-packet-button]').click({ force: true })
    cy.get('[data-test=start-button]').click({ force: true })
    cy.wait(1000) // wait for the first packet to come in

    cy.get('[data-test=dump-component-open-settings]').click({ force: true })
    cy.wait(1000) // give the dialog time to open
    cy.get('[data-test=display-settings-card]').should('be.visible')
    cy.get('[data-test=dump-component-settings-format-ascii]').check({
      force: true,
    })
    cy.get('[data-test=dump-component-settings-newest-top]').check({
      force: true,
    })
    cy.get('[data-test=dump-component-settings-show-address]').check({
      force: true,
    })
    cy.get('[data-test=dump-component-settings-show-timestamp]').check({
      force: true,
    })

    // check number input validation
    cy.get('[data-test=dump-component-settings-num-bytes]')
      .clear({ force: true })
      .type('0{enter}')
    cy.get('[data-test=dump-component-settings-num-bytes]')
      .invoke('val')
      .then((val) => {
        expect(val).to.eq('1')
      })
    cy.get('[data-test=dump-component-settings-num-packets]')
      .clear({ force: true })
      .type('0{enter}')
    cy.get('[data-test=dump-component-settings-num-packets]')
      .invoke('val')
      .then((val) => {
        expect(val).to.eq('1')
        cy.get('[data-test=dump-component-settings-num-packets]')
          .clear({ force: true })
          .type('101{enter}') // bigger than HISTORY_MAX_SIZE
        return cy
          .get('[data-test=dump-component-settings-num-packets]')
          .invoke('val')
      })
      .then((val) => {
        expect(val).to.eq('100')
      })
  })

  it('downloads a file', () => {
    cy.skipOn('electron') // because the readDownloads task doesn't work for some reason
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]').click({ force: true }).wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.wait(1000) // wait for the first packet to come in

    cy.get('[data-test=dump-component-play-pause]')
      .click({ force: true })
      .wait(1000)
    cy.get('[data-test=dump-component-download]')
      .click({ force: true })
      .wait(1000)
    cy.wait(800)
    cy.task('readDownloads')
      .then((files) => {
        expect(files.length).to.eq(1)
        return Promise.all([
          cy.readFile(files[0]),
          cy.get('[data-test=dump-component-text-area]').invoke('val'),
        ])
      })
      .then(([fileContents, inputContents]) => {
        expect(fileContents).to.eq(inputContents)
      })
  })

  it('validates start and end time inputs', () => {
    // validate start date
    cy.get('[data-test=startDate]').clear({ force: true })
    cy.get('.container').should('contain', 'Required')
    cy.get('[data-test=startDate]').clear({ force: true }).type('2020-01-01')
    cy.get('.container').should('not.contain', 'Invalid')
    // validate start time
    cy.get('[data-test=startTime]').clear({ force: true })
    cy.get('.container').should('contain', 'Required')
    cy.get('[data-test=startTime]').clear({ force: true }).type('12:15:15')
    cy.get('.container').should('not.contain', 'Invalid')

    // validate end date
    cy.get('[data-test=endDate]').clear({ force: true }).type('2020-01-01')
    cy.get('.container').should('not.contain', 'Invalid')
    // validate end time
    cy.get('[data-test=endTime]').clear({ force: true }).type('12:15:15')
    cy.get('.container').should('not.contain', 'Invalid')
  })

  it('validates start and end time values', () => {
    // validate future start date
    cy.get('[data-test=startDate]').clear({ force: true }).type('4000-01-01') // If this version of COSMOS is still used 2000 years from now, this test will need to be updated
    cy.get('[data-test=startTime]').clear({ force: true }).type('12:15:15')
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.get('.warning').should('contain', 'Start date/time is in the future!')

    // validate start/end time equal to each other
    cy.get('[data-test=startDate]').clear({ force: true }).type('2020-01-01')
    cy.get('[data-test=startTime]').clear({ force: true }).type('12:15:15')
    cy.get('[data-test=endDate]').clear({ force: true }).type('2020-01-01')
    cy.get('[data-test=endTime]').clear({ force: true }).type('12:15:15')
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.get('.warning').should(
      'contain',
      'Start date/time is equal to end date/time!'
    )

    // validate future end date
    cy.get('[data-test=startDate]').clear({ force: true }).type('2020-01-01')
    cy.get('[data-test=startTime]').clear({ force: true }).type('12:15:15')
    cy.get('[data-test=endDate]').clear({ force: true }).type('4000-01-01')
    cy.get('[data-test=endTime]').clear({ force: true }).type('12:15:15')
    cy.get('[data-test=start-button]').click({ force: true }).wait(1000)
    cy.get('.warning').should(
      'contain',
      'Note: End date/time is greater than current date/time. Data will continue to stream in real-time until 4000-01-01 12:15:15 is reached.'
    )
  })

  it('saves and loads the configuration', () => {
    cy.get('[data-test=new-tab]').click({ force: true }).wait(1000)
    cy.get('[data-test=new-packet]').click({ force: true }).wait(1000)
    cy.selectTargetPacketItem('INST', 'ADCS')
    cy.wait(1000)
    cy.get('[data-test=add-packet-button]').click({ force: true }).wait(1000)
    let config = 'spec' + Math.floor(Math.random() * 10000)
    cy.get('.v-toolbar').contains('File').click({ force: true }).wait(1000)
    cy.contains('Save Configuration').click({ force: true }).wait(1000)
    cy.get('.v-dialog:visible').within(() => {
      cy.get('[data-test=name-input-save-config-dialog]')
        .clear({ force: true })
        .type(config)
      cy.contains('Ok').click({ force: true }).wait(1000)
    })
    cy.get('.v-dialog:visible').should('not.exist')
    // Verify we get a warning if trying to save over existing
    cy.get('.v-toolbar').contains('File').click({ force: true }).wait(1000)
    cy.contains('Save Configuration').click({ force: true }).wait(1000)
    cy.get('.v-dialog:visible').within(() => {
      cy.get('[data-test=name-input-save-config-dialog]')
        .clear({ force: true })
        .type(config)
      cy.contains('Ok').click({ force: true }).wait(1000)
      cy.contains("'" + config + "' already exists")
      cy.contains('Cancel').click({ force: true }).wait(1000)
    })
    cy.get('.v-dialog:visible').should('not.exist')
    // Totally refresh the page
    cy.visit('/tools/dataviewer')
    cy.hideNav()
    cy.wait(1000)
    // the last config should open automatically
    cy.get('.v-window-item > .v-card > .v-card__title').should(
      'contain',
      'INST ADCS'
    )

    cy.get('.v-toolbar').contains('File').click({ force: true }).wait(1000)
    cy.contains('Open Configuration').click({ force: true }).wait(1000)
    cy.get('.v-dialog:visible').within(() => {
      // Try to click OK without anything selected
      cy.contains('Ok').click({ force: true }).wait(1000)
      cy.contains('Select a configuration')
      cy.contains(config).click({ force: true }).wait(1000)
      cy.contains('Ok').click({ force: true }).wait(1000)
    })
    // Verify we're back
    cy.get('.v-window-item > .v-card > .v-card__title').should(
      'contain',
      'INST ADCS'
    )
    // Delete this test configuation
    cy.get('.v-toolbar').contains('File').click({ force: true }).wait(1000)
    cy.contains('Open Configuration').click({ force: true }).wait(1000)
    cy.get('.v-dialog:visible').within(() => {
      cy.contains(config)
        .parents('.v-list-item')
        .eq(0)
        .within(() => {
          cy.get('button').click({ force: true }).wait(1000)
        })
      cy.contains('Cancel').click({ force: true }).wait(1000)
    })
  })
})
