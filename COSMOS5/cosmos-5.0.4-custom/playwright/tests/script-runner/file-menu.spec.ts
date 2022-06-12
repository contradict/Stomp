/*
# Copyright 2022 Ball Aerospace & Technologies Corp.
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

// @ts-check
import { test, expect } from 'playwright-test-coverage'
import { Utilities } from '../../utilities'

let utils
test.beforeEach(async ({ page }) => {
  await page.goto('/tools/scriptrunner')
  await expect(page.locator('.v-app-bar')).toContainText('Script Runner')
  await page.locator('.v-app-bar__nav-icon').click()
  utils = new Utilities(page)
})

test('clears the editor on File->New', async ({ page }) => {
  // Have to fill on an editable area like the textarea
  await page.locator('textarea').fill('this is a test')
  // But can't check on the textarea because it has an input
  await expect(page.locator('#editor')).toContainText('this is a test')
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=New File').click()
  await expect(page.locator('#editor')).not.toContainText('this is a test')
})

test('open a file', async ({ page }) => {
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Open File').click()
  await utils.sleep(1000)
  await page.locator('[data-test=file-open-save-search]').type('dis')
  await utils.sleep(500)
  await page.locator('[data-test=file-open-save-search]').type('connect')
  await page.locator('text=disconnect >> nth=0').click() // nth=0 because INST, INST2
  await page.locator('[data-test=file-open-save-submit-btn]').click()
  expect(await page.locator('#sr-controls')).toContainText(`INST/procedures/disconnect.rb`)
})

test('handles File->Save new file', async ({ page }) => {
  await page.locator('textarea').fill('puts "File Save new File"')
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Save File').click()
  // New files automatically open File Save As
  await page.locator('text=File Save As')
  await page.locator('[data-test=file-open-save-filename]').fill('save_new.rb')
  await page.locator('text=temp.rb is not a valid filename')
  // This selector (generated by playwright) sucks but not sure what else works
  await page.locator('text=EXAMPLEINSTINST2SYSTEMTEMPLATED >> button').nth(1).click()
  await page.locator('text=procedures').click()
  await page.locator('[data-test=file-open-save-filename]').click()
  await page.type('[data-test=file-open-save-filename]', '/save_new.rb')
  await page.locator('[data-test=file-open-save-submit-btn]').click()
  expect(await page.locator('#sr-controls')).toContainText('INST/procedures/save_new.rb')

  // Delete the file
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Delete File').click()
  await page.locator('text=Permanently delete file')
  await page.locator('button:has-text("Delete")').click()
})

test('handles File Save overwrite', async ({ page }) => {
  await page.locator('textarea').fill('puts "File Save overwrite"')
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Save File').click()
  await page
    .locator('[data-test=file-open-save-filename]')
    .fill('INST/procedures/save_overwrite.rb')
  await page.locator('[data-test=file-open-save-submit-btn]').click()
  expect(await page.locator('#sr-controls')).toContainText('INST/procedures/save_overwrite.rb')

  await page.locator('textarea').fill('# comment1')
  // TODO: Check for * by filename ... not currently implemented
  // expect(await page.locator('[data-test=filename]')).toContainText('INST/procedures/temp.rb *')
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Save File').click()
  // TODO: Check that * by filename goes away ... not currently implemented
  // expect(await page.locator('[data-test=filename]')).toContainText('INST/procedures/temp.rb')
  // expect(await page.locator('[data-test=filename]')).not.toContainText('*')
  await page.locator('textarea').fill('# comment2')
  // TODO: Check for * by filename ... not currently implemented
  // expect(await page.locator('[data-test=filename]')).toContainText('INST/procedures/temp.rb *')
  await page.locator('textarea').press('Control+s') // Ctrl-S save
  // TODO: Check that * by filename goes away ... not currently implemented
  // expect(await page.locator('[data-test=filename]')).toContainText('INST/procedures/temp.rb')
  // expect(await page.locator('[data-test=filename]')).not.toContainText('*')

  // File->Save As
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Save As...').click()
  await page.locator('text=INST/procedures/save_overwrite.rb')
  await page.locator('[data-test=file-open-save-submit-btn]').click()
  // Confirmation dialog
  await page.locator('text=Are you sure you want to overwrite').click()
  await page.locator('button:has-text("Overwrite")').click()

  // Delete the file
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Delete File').click()
  await page.locator('text=Permanently delete file')
  await page.locator('button:has-text("Delete")').click()
})

test('handles Download', async ({ page }) => {
  await page.locator('textarea').fill('download this')
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Save File').click()
  await page.fill('[data-test=file-open-save-filename]', 'INST/download.txt')
  await page.locator('[data-test=file-open-save-submit-btn]').click()
  expect(await page.locator('#sr-controls')).toContainText('INST/download.txt')
  // Download the file
  await page.locator('[data-test=script-runner-file]').click()
  await utils.download(page, '[data-test=script-runner-file-download]', function (contents) {
    expect(contents).toContain('download this')
  })

  // Delete the file
  await page.locator('[data-test=script-runner-file]').click()
  await page.locator('text=Delete File').click()
  await page.locator('text=Permanently delete file')
  await page.locator('button:has-text("Delete")').click()
})