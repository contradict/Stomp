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

import Api from '@cosmosc2/tool-common/src/services/api'

const getKeywords = (type) => {
  return Api.get(`/cosmos-api/autocomplete/keywords/${type}`)
}

const getAutocompleteData = (type) => {
  return Api.get(`/cosmos-api/autocomplete/data/${type}`)
}

// This should probably go in some higher up util library thing place
const groupBy = (array, lambda) => {
  return array.reduce((groups, item) => {
    const key = lambda(item)
    if (groups[key]) {
      groups[key].push(item)
    } else {
      groups[key] = [item]
    }
    return groups
  }, {})
}

export { getKeywords, getAutocompleteData, groupBy }
