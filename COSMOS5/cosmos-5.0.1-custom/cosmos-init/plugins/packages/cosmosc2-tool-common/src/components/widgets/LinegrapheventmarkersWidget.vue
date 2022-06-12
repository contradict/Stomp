<!--
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
#
-->

<template>
  <graph
    :ref="'graph' + id"
    :id="id"
    :state="state"
    :selectedGraphId="id"
    :secondsGraphed="secondsGraphed"
    :pointsSaved="pointsSaved"
    :pointsGraphed="pointsGraphed"
    :initialItems="items"
    :height="size.height"
    :width="size.width"
    hide-system-bar
    hide-overview
  />
</template>

<script>
import Widget from './Widget'
import Graph from '../Graph.vue'

const valueType = 'CONVERTED'

export default {
  components: {
    Graph,
  },
  mixins: [Widget],
  data: function () {
    return {
      id: Math.floor(Math.random() * 100000000000), // Unique-ish
      state: 'start',
      items: [],
      secondsGraphed: 1000,
      pointsSaved: 1000000,
      pointsGraphed: 1000,
      size: {
        height: 300,
        width: 400,
      },
    }
  },
  created: function () {
    // Look through the settings and see if we're a NAMED_WIDGET
    this.settings.forEach((setting) => {
      if (setting[0] === 'NAMED_WIDGET') {
        setting[2].setNamedWidget(setting[1], this)
      }
    })

    this.settings.forEach((setting) => {
      switch (setting[0]) {
        case 'ITEM':
          this.items.push({
            targetName: setting[1],
            packetName: setting[2],
            itemName: setting[3],
            plotType: setting[4],
            scaleName: setting[5],
            valueType,
          })
          break
        case 'SECONDSGRAPHED':
          this.secondsGraphed = parseInt(setting[1])
          break
        case 'POINTSSAVED':
          this.pointsSaved = parseInt(setting[1])
          break
        case 'POINTSGRAPHED':
          this.pointsGraphed = parseInt(setting[1])
          break
        case 'SIZE':
          this.size.width = parseInt(setting[1])
          this.size.height = parseInt(setting[2])
          break
      }
    })
  },
  methods: {
    getState: function() {
      let graphRef = "graph" + this.id;
      return this.$refs[graphRef].state;
    },
    setState: function(new_state) {
      let graphRef = "graph" + this.id;
      this.$refs[graphRef].state = new_state;
    },
  },
}
</script>
