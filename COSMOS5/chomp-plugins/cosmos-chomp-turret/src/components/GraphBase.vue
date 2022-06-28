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
-->

<template>
  <div>
    <v-card @click.native="$emit('click')">
      <v-system-bar
        :class="selectedGraphId === id ? 'active' : 'inactive'"
        v-show="!hideSystemBar"
      >
        <div v-show="errors.length !== 0" class="mx-2">
          <v-tooltip top>
            <template v-slot:activator="{ on, attrs }">
              <div v-on="on" v-bind="attrs">
                <v-icon data-test="errorGraphIcon" @click="errorDialog = true">
                  mdi-alert
                </v-icon>
              </div>
            </template>
            <span> Errors </span>
          </v-tooltip>
        </div>

        <v-spacer />
        <span>{{ title }}</span>
        <v-spacer />

        <div v-show="expand">
          <v-tooltip top>
            <template v-slot:activator="{ on, attrs }">
              <div v-on="on" v-bind="attrs">
                <v-icon
                  data-test="collapseAll"
                  v-show="calcFullSize"
                  @click="collapseAll"
                >
                  mdi-arrow-collapse
                </v-icon>
                <v-icon
                  data-test="expandAll"
                  v-show="!calcFullSize"
                  @click="expandAll"
                >
                  mdi-arrow-expand
                </v-icon>
              </div>
            </template>
            <span v-show="calcFullSize"> Collapse </span>
            <span v-show="!calcFullSize"> Expand </span>
          </v-tooltip>
        </div>

        <div v-show="expand">
          <v-tooltip top>
            <template v-slot:activator="{ on, attrs }">
              <div v-on="on" v-bind="attrs">
                <v-icon
                  data-test="collapseWidth"
                  v-show="fullWidth"
                  @click="collapseWidth"
                >
                  mdi-arrow-collapse-horizontal
                </v-icon>
                <v-icon
                  data-test="expandWidth"
                  v-show="!fullWidth"
                  @click="expandWidth"
                >
                  mdi-arrow-expand-horizontal
                </v-icon>
              </div>
            </template>
            <span v-show="fullWidth"> Collapse Width </span>
            <span v-show="!fullWidth"> Expand Width </span>
          </v-tooltip>
        </div>

        <div v-show="expand">
          <v-tooltip top>
            <template v-slot:activator="{ on, attrs }">
              <div v-on="on" v-bind="attrs">
                <v-icon
                  data-test="collapseVertical"
                  v-show="fullHeight"
                  @click="collapseHeight"
                >
                  mdi-arrow-collapse-vertical
                </v-icon>
                <v-icon
                  data-test="expandVertical"
                  v-show="!fullHeight"
                  @click="expandHeight"
                >
                  mdi-arrow-expand-vertical
                </v-icon>
              </div>
            </template>
            <span v-show="fullHeight"> Collapse Height </span>
            <span v-show="!fullHeight"> Expand Height </span>
          </v-tooltip>
        </div>

        <v-tooltip top>
          <template v-slot:activator="{ on, attrs }">
            <div v-on="on" v-bind="attrs">
              <v-icon
                data-test="minimizeScreenIcon"
                @click="minMaxTransition"
                v-show="expand"
              >
                mdi-window-minimize
              </v-icon>
              <v-icon
                data-test="maximizeScreenIcon"
                @click="minMaxTransition"
                v-show="!expand"
              >
                mdi-window-maximize
              </v-icon>
            </div>
          </template>
          <span v-show="expand"> Minimize </span>
          <span v-show="!expand"> Maximize </span>
        </v-tooltip>

        <v-tooltip top>
          <template v-slot:activator="{ on, attrs }">
            <div v-on="on" v-bind="attrs">
              <v-icon data-test="closeGraphIcon" @click="$emit('close-graph')">
                mdi-close-box
              </v-icon>
            </div>
          </template>
          <span> Close </span>
        </v-tooltip>
      </v-system-bar>

      <v-expand-transition>
        <div class="pa-1" id="chart" ref="chart" v-show="expand">
          <div :id="`chart${id}`"></div>
        </div>
      </v-expand-transition>
    </v-card>

    <!-- Error dialog -->
    <v-dialog v-model="errorDialog" max-width="600">
      <v-system-bar>
        <v-spacer />
        <span>Errors</span>
        <v-spacer />
      </v-system-bar>
      <v-card class="pa-3">
        <v-row dense>
          <v-text-field
            readonly
            hide-details
            v-model="title"
            class="pb-2"
            label="Graph Title"
          />
        </v-row>
        <v-row class="my-3">
          <v-textarea readonly rows="8" :value="error" />
        </v-row>
        <v-row>
          <v-btn block @click="clearErrors"> Clear </v-btn>
        </v-row>
      </v-card>
    </v-dialog>
  </div>
</template>

<script>
import uPlot from "uplot";
import bs from "binary-search";
import { toDate, format, getTime } from "date-fns";
import Cable from "../services/cable.js";

require("uplot/dist/uPlot.min.css");

// BB MJS: Move this out of this vue file

function eventMarkers(opts) {
	opts = opts || {};
  
  let labels = [];
	let textFill = "black";
	let rectFill = "white";
	let rectStroke = "black";
	let showLabels = opts.showLabels == null ? true: opts.showLabels;
	let labelsAlign = opts.labelsAlign == null ? "top" : opts.labelsAlign;

  function drawPathLabels(u, seriesIdx) {
		let rectPadding = 7;

		let s = u.series[seriesIdx];

		textFill = s._stroke == null ? textFill : s._stroke;
		rectFill = s._fill == null ? rectFill : s._fill;
		rectStroke = s._stroke == null ? rectStroke : s._stroke;

		u.ctx.fillStyle = textFill;
		u.ctx.textAlign = "center";
		u.ctx.textBaseline = "center";

		labels.forEach((label) => {
			let text = u.ctx.measureText(label.text);
	
			let textWidth = text.width;
			let textHeight = text.actualBoundingBoxDescent + text.actualBoundingBoxAscent;
	
			let rectWidth = textWidth + (rectPadding * 2);
			let rectHeight = textHeight + (rectPadding * 2);
	
			let yOffAlign = text.actualBoundingBoxAscent + rectPadding;
	
			if (label.align == "center")
			{
				yOffAlign = rectHeight;
			}
			else if (label.align == "bottom")
			{
				yOffAlign = -(text.actualBoundingBoxDescent + rectPadding);
			}
	
			let textCenterX = label.x;
			let textCenterY = label.y + yOffAlign;
	
			let rectTop = textCenterX - (rectWidth / 2);
			let rectLeft = textCenterY - (rectHeight / 2);
	
			u.ctx.fillStyle = rectFill;
			u.ctx.strokeStyle = rectStroke;
	
			u.ctx.fillRect(rectTop, rectLeft, rectWidth, rectHeight);
			u.ctx.strokeRect(rectTop, rectLeft, rectWidth, rectHeight);
	
			u.ctx.fillStyle = textFill;
			u.ctx.fillText(label.text, textCenterX, textCenterY);
		});
	}

	return (u, seriesIdx, idx0, idx1) => {
		return uPlot.orient(u, seriesIdx, (series, dataX, dataY, scaleX, scaleY, valToPosX, valToPosY, xOff, yOff, xDim, yDim) => {
			
			labels = [];
	
			let pxRound = series.pxRound;
			const _paths = {stroke: new Path2D(), fill: null, text: drawPathLabels, clip: null, band: null, gaps: null, flags: uPlot.BAND_CLIP_FILL};
			const stroke = _paths.stroke;

			let yBottom = yOff + yDim;
			let yTop = yOff;

			let labelPadding = 6;
			let yLabel = yTop + labelPadding;

			if (labelsAlign == "bottom")
			{
				yLabel = yBottom - labelPadding;
			}
			else if (labelsAlign == "center")
			{
				yLabel = (yBottom - yTop) / 2;
			}

			for (let i = idx0; i <= idx1; i++) {
				let yEventName = dataY[i];

				if (yEventName == null) {
					continue;
				}
				let x = pxRound(valToPosX(dataX[i], scaleX, xDim, xOff));

				stroke.moveTo(x, yBottom);
				stroke.lineTo(x, yTop);

				if (showLabels)
				{	
					let labelElement = {text: dataY[i], align: labelsAlign, x: x, y: yLabel};
					labels.push(labelElement);
				}
			}

			_paths.gaps = null;
			_paths.fill = null;
			_paths.clip = null;
			_paths.band = null;

			return _paths;
		});
	};
}

export default {
  props: {
    id: {
      type: Number,
      required: true,
    },
    selectedGraphId: {
      type: Number,
      // Not required because we pass null
    },
    state: {
      type: String,
      required: true,
    },
    // start time in nanoseconds to start the graph
    // this allows multiple graphs to be synchronized
    startTime: {
      type: Number,
    },
    secondsGraphed: {
      type: Number,
      required: true,
    },
    pointsSaved: {
      type: Number,
      required: true,
    },
    pointsGraphed: {
      type: Number,
      required: true,
    },
    hideSystemBar: {
      type: Boolean,
      default: false,
    },
    initialItems: {
      type: Array,
    },
    axes: {
      type: Array,
    },
    axesScales: {
      type: Object,
    },
    // These allow the parent to force a specific height and/or width
    height: {
      type: Number,
    },
    width: {
      type: Number,
    },
  },
  data() {
    return {
      itemHeaders: [
        { text: 'Target Name', value: 'targetName' },
        { text: 'Packet Name', value: 'packetName' },
        { text: 'Item Name', value: 'itemName' },
        { text: 'Actions', value: 'actions', sortable: false },
      ],
      valueTypes: ['CONVERTED', 'RAW'],
      active: true,
      expand: true,
      fullWidth: false,
      fullHeight: true,
      graph: null,
      editGraph: false,
      editGraphMenu: false,
      editGraphMenuX: 0,
      editGraphMenuY: 0,
      editItem: false,
      itemMenu: false,
      itemMenuX: 0,
      itemMenuY: 0,
      selectedItem: null,
      title: '',
      data: [[]],
      graphMinY: '',
      graphMaxY: '',
      graphStartDateTime: this.startTime,
      graphEndDateTime: null,
      indexes: {},
      items: this.initialItems || [],
      scales: null,
      drawInterval: null,
      zoomChart: false,
      cable: new Cable(),
      subscription: null,
      needToUpdate: false,
      errorDialog: false,
      errors: [],
      colors: [
        'blue',
        'red',
        'green',
        'darkorange',
        'purple',
        'cornflowerblue',
        'lime',
        'gold',
        'hotpink',
        'tan',
        'cyan',
        'peru',
        'maroon',
        'coral',
        'navy',
        'teal',
        'brown',
        'crimson',
        'lightblue',
        'black',
      ],
    }
  },
  computed: {
    calcFullSize: function () {
      return this.fullWidth || this.fullHeight;
    },
    error: function () {
      if (this.errorDialog && this.errors.length > 0) {
        return JSON.stringify(this.errors, null, 4);
      }
      return null;
    },
  },
  created() {
    this.title = `Graph ${this.id}`
    for (const [index, item] of this.items.entries()) {
      this.data.push([]) // initialize the empty data arrays
      this.indexes[this.subscriptionKey(item)] = index + 1
    }

    this.scales = {
      "x": {
        range(u, dataMin, dataMax) {
          if (dataMin == null) return [1566453600, 1566497660]
          return [dataMin, dataMax]
        },
      },
      "y": {
        range(u, dataMin, dataMax) {
          if (dataMin == null) return [-100, 100]
          return uPlot.rangeNum(dataMin, dataMax, 0.1, true)
        },
      },
    }
  },
  mounted() {
    const { chartSeries } = this.items.reduce(
      (seriesObj, item) => {
        const commonProps = {
          spanGaps: true,
          stroke: this.colors.shift(),
        }
        item.plotType ||= 'LINEAR'
        item.scaleName ||= 'y'

        if (item.plotType == 'EVENT_MARKER')
        {
          seriesObj.chartSeries.push({
            ...commonProps,
            paths: eventMarkers({showLabels: true, labelsAlign: "top"}),
            item: item,
            label: item.itemName,
            scale: item.scaleName,
            auto: false,
            width: 2,
            value: (self, rawValue) =>
              rawValue == null ? '--' : rawValue,
          })
        }
        else
        {
          seriesObj.chartSeries.push({
            ...commonProps,
            item: item,
            label: item.itemName,
            scale: item.scaleName,
            value: (self, rawValue) =>
              rawValue == null ? '--' : rawValue.toFixed(2),
          })
        }
        return seriesObj
      },
      { chartSeries: [] }
    )

    for(var axisScale in this.axesScales) 
    {
      if (this.scales.hasOwnProperty(axisScale))
      {
        delete this.scales[axisScale];
      }
      
      this.scales[axisScale] = this.axesScales[axisScale];
    }

    let chartOpts = {
      ...this.getSize('chart'),
      scales: this.scales,
      axes: [
        {
          stroke: "white",
          grid: {
            show: true,
            stroke: "rgba(255, 255, 255, .1)",
            width: 2,
          },
        },
        {
          size: 70, // This size supports values up to 99 million
          stroke: "white",
          grid: {
            show: true,
            stroke: "rgba(255, 255, 255, .1)",
            width: 2,
          },
        },
        ...this.axes,
      ],
      series: [
        {
          label: 'Time',
          value: (u, v) =>
            // Convert the unix timestamp into a formatted date / time
            v == null ? '--' : format(toDate(v * 1000), 'yyyy-MM-dd HH:mm:ss'),
        },
        ...chartSeries,
      ],
      cursor: {
        drag: {
          x: true,
          y: false,
        },
        // Sync the cursor across graphs so mouseovers are synced
        sync: {
          key: 'cosmos',
          // setSeries links graphs so clicking an item to hide it also hides the other graph item
          // setSeries: true,
        },
      },
      hooks: {
        ready: [
          (u) => {
            let clientX
            let clientY
            let canvas = u.root.querySelector('canvas')
            canvas.addEventListener('contextmenu', (e) => {
              e.preventDefault()
              this.editGraphMenuX = e.clientX
              this.editGraphMenuY = e.clientY
              this.editGraphMenu = true
            })
            let legend = u.root.querySelector('.u-legend')
            legend.addEventListener('contextmenu', (e) => {
              e.preventDefault()
              this.itemMenuX = e.clientX
              this.itemMenuY = e.clientY
              // Grab the closest series and then figure out which index it is
              let seriesEl = e.target.closest('.u-series')
              let seriesIdx = Array.prototype.slice
                .call(legend.childNodes)
                .indexOf(seriesEl)
              let series = u.series[seriesIdx]
              if (series.item) {
                this.selectedItem = series.item
                this.itemMenu = true
              }
              return false
            })
          },
        ],
      },
    }

    this.graph = new uPlot(
      chartOpts,
      this.data,
      document.getElementById(`chart${this.id}`)
    )

    // Allow the charts to dynamically resize when the window resizes
    window.addEventListener('resize', this.handleResize)

    if (this.state !== 'stop') {
      this.subscribe()
    }
  },
  beforeDestroy: function () {
    if (this.subscription) {
      this.subscription.unsubscribe()
    }
    this.cable.disconnect()
    window.removeEventListener('resize', this.handleResize)
  },
  watch: {
    state: function (newState, oldState) {
      switch (newState) {
        case 'start':
          // Only subscribe if we were previously stopped
          // If we were paused we do nothing ... see the data function
          if (oldState === 'stop') {
            this.subscribe()
          }
          break
        // case 'pause': Nothing to do ... see the data function
        case 'stop':
          this.subscription.unsubscribe()
          this.subscription = null
          break
      }
    },
    data: function (newData, oldData) {
      // Ignore changes to the data while we're paused
      if (this.state === 'pause') {
        return
      }
      this.graph.setData(newData)
      let max = newData[0][newData[0].length - 1]
      let ptsMin = newData[0][newData[0].length - this.pointsGraphed]
      let min = newData[0][0]
      if (min < max - this.secondsGraphed) {
        min = max - this.secondsGraphed
      }
      if (ptsMin > min) {
        min = ptsMin
      }
      this.graph.setScale('x', { min, max })
    },
    graphMinY: function (newVal, oldVal) {
      let val = parseFloat(newVal)
      if (!isNaN(val)) {
        this.graphMinY = val
      }
      this.setGraphRange()
    },
    graphMaxY: function (newVal, oldVal) {
      let val = parseFloat(newVal)
      if (!isNaN(val)) {
        this.graphMaxY = val
      }
      this.setGraphRange()
    },
    graphStartDateTime: function (val) {
      this.needToUpdate = true
      if (val && typeof val === 'string') {
        this.graphStartDateTime =
          new Date(this.graphStartDateTime).getTime() * 1_000_000
      }
    },
    graphEndDateTime: function (val) {
      this.needToUpdate = true
      if (val && typeof val === 'string') {
        this.graphEndDateTime =
          new Date(this.graphEndDateTime).getTime() * 1_000_000
      }
    },
  },
  methods: {
    clearErrors: function () {
      this.errors = []
    },
    handleResize: function () {
      // TODO: Should this method be throttled?
      this.graph.setSize(this.getSize('chart'))
    },
    resize: function () {
      this.graph.setSize(this.getSize('chart'))
      this.$emit('resize', this.id)
    },
    expandAll: function () {
      this.fullWidth = true
      this.fullHeight = true
      this.resize()
    },
    collapseAll: function () {
      this.fullWidth = false
      this.fullHeight = false
      this.resize()
    },
    expandWidth: function () {
      this.fullWidth = true
      this.resize()
    },
    collapseWidth: function () {
      this.fullWidth = false
      this.resize()
    },
    expandHeight: function () {
      this.fullHeight = true
      this.resize()
    },
    collapseHeight: function () {
      this.fullHeight = false
      this.resize()
    },
    minMaxTransition: function () {
      this.expand = !this.expand
      this.$emit('min-max-graph', this.id)
    },
    setGraphRange: function () {
      let pad = 0.1
      if (
        this.graphMinY ||
        this.graphMinY === 0 ||
        this.graphMaxY ||
        this.graphMaxY === 0
      ) {
        pad = 0
      }
      this.graph.scales.y.range = (u, dataMin, dataMax) => {
        let min = dataMin
        if (this.graphMinY || this.graphMinY === 0) {
          min = this.graphMinY
        }
        let max = dataMax
        if (this.graphMaxY || this.graphMaxY === 0) {
          max = this.graphMaxY
        }
        return uPlot.rangeNum(min, max, pad, true)
      }
    },
    subscribe: function (endTime = null) {

      // eslint-disable-next-line no-console
      console.log("Subscribe!");

      this.cable
        .createSubscription('StreamingChannel', localStorage.scope, {
          received: (data) => this.received(data),
          connected: () => {
            // eslint-disable-next-line no-console
            console.log("Subscribe - connected!");
            this.addItemsToSubscription(this.items, endTime)
          },
        disconnected: () => {
          // eslint-disable-next-line no-console
          console.log("Subscribe - disconnected!");
          this.errors.push({
            type: 'disconnected',
            message: 'COSMOS backend connection disconnected',
            time: new Date().getTime()
          })
        },
        rejected: () => {
          // eslint-disable-next-line no-console
          console.log("Subscribe - rejected!");
          this.errors.push({
            type: 'rejected',
            message: 'COSMOS backend connection rejected',
            time: new Date().getTime()
          })
        },
      })
      .then((subscription) => {
        this.subscription = subscription
      })
    },
    getSize: function (type) {
      const navDrawer = document.getElementById('cosmos-nav-drawer')
      const navDrawerWidth = navDrawer.classList.contains(
        'v-navigation-drawer--open'
      )
        ? navDrawer.clientWidth
        : 0
      const viewWidth =
        Math.max(document.documentElement.clientWidth, window.innerWidth || 0) -
        navDrawerWidth
      const viewHeight = 
        Math.max(document.documentElement.clientHeight, window.innerHeight || 0)

      const chooser = document.getElementsByClassName('c-chooser')[0]
      let height = 100
      if (chooser) {
        // Height of chart is viewportSize - chooser - fudge factor (primarily padding)
        height = viewHeight - chooser.clientHeight - height - 190
        if (!this.fullHeight) {
          height = height / 2.0 + 10 // 5px padding top and bottom
        }
      }
      let width = viewWidth - 60 // 30px padding left and right
      if (!this.fullWidth) {
        width = width / 2.0 - 10 // 5px padding left and right
      }
      return {
        width: this.width || width,
        height: this.height || height,
      }
    },
    addItemsToSubscription: function (
      itemArray = this.items,
      endTime = this.graphEndDateTime
    ) {
      // eslint-disable-next-line no-console
      console.log("addItemsToSub");

      if (this.subscription) {

        // eslint-disable-next-line no-console
        console.log("addItemsToSub - valid subscription");

        CosmosAuth.updateToken(CosmosAuth.defaultMinValidity).then(() => {
          this.subscription.perform('add', {
            scope: localStorage.scope,
            mode: 'DECOM',
            token: "123Happy!",
            items: itemArray.map(this.subscriptionKey),
            start_time: this.graphStartDateTime,
            end_time: endTime,
          })
        })
      }
    },
    removeItemsFromSubscription: function (itemArray = this.items) {
      if (this.subscription) {
        this.subscription.perform('remove', {
          scope: localStorage.scope,
          items: itemArray.map(this.subscriptionKey),
        })
      }
    },
    reorderIndexes: function (key) {
      let index = this.indexes[key]
      delete this.indexes[key]
      for (var i in this.indexes) {
        if (this.indexes[i] > index) {
          this.indexes[i] -= 1
        }
      }
      return index
    },
    received: function (json_data) {
      // TODO: Shouldn't get errors but should we handle this every time?
      // if (json_data.error) {
      //   console.log(json_data.error)
      //   return
      // }
      let data = JSON.parse(json_data)
      for (let i = 0; i < data.length; i++) {
        let time = data[i].time / 1_000_000_000.0 // Time in seconds
        let length = data[0].length
        if (length == 0 || time > data[0][length - 1]) {
          // Nominal case - append new data to end
          for (let j = 0; j < this.data.length; j++) {
            this.data[j].push(null)
          }
          this.set_data_at_index(this.data[0].length - 1, time, data[i])
        } else {
          let index = bs(this.data[0], time, this.bs_comparator)
          if (index >= 0) {
            // Found the slot in the existing data
            this.set_data_at_index(index, time, data[i])
          } else {
            // Insert a new null slot at the ideal index
            let ideal_index = -index - 1
            for (let j = 0; j < this.data.length; j++) {
              this.data[j].splice(ideal_index, 0, null)
            }
            this.set_data_at_index(ideal_index, time, data[i])
          }
        }
      }
      // If we weren't passed a startTime notify grapher of our start
      if (this.startTime === null) {
        this.graphStartDateTime = this.data[0][0] * 1_000_000_000
        this.$emit('started', this.graphStartDateTime)
      }
    },
    bs_comparator: function (element, needle) {
      return element - needle
    },
    set_data_at_index: function (index, time, new_data) {
      this.data[0][index] = time
      for (const [key, value] of Object.entries(new_data)) {
        if (key == 'time') {
          continue
        }
        let key_index = this.indexes[key]
        if (key_index) {
          let array = this.data[key_index]
          if (!value.raw) {
            array[index] = value
          } else {
            array[index] = null
          }
        }
      }
    },
    subscriptionKey: function (item) {
      return `TLM__${item.targetName}__${item.packetName}__${item.itemName}__${item.valueType}`
    },
  },
}
</script>

<style scoped>
#chart {
  background-color: var(--v-tertiary-darken2);
}
#chart >>> .u-legend {
  text-align: left;
}
#chart >>> .u-inline {
  max-width: fit-content;
}
/* TODO: Get this to work with white theme, values would be 0 in white */
#chart >>> .u-select {
  background-color: rgba(255, 255, 255, 0.07);
}
</style>
