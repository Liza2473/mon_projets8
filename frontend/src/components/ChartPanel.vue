<template>
  <div class="chart-wrapper">

    <div class="chart-controls">
      <div class="ctrl-group">
        <span class="ctrl-label">Afficher :</span>
        <button class="chip" :class="{ 'chip-green': showTemp }" @click="showTemp = !showTemp">🌡 Température</button>
        <button class="chip" :class="{ 'chip-blue': showPres }"  @click="showPres = !showPres">📊 Pression</button>
      </div>
      <div class="ctrl-group">
        <span class="ctrl-label">Type :</span>
        <button v-for="t in chartTypes" :key="t.value" class="chip" :class="{ 'chip-accent': chartType === t.value }" @click="chartType = t.value">{{ t.label }}</button>
      </div>
      <div class="ctrl-group">
        <span class="ctrl-label">Vue :</span>
        <button class="chip" :class="{ 'chip-accent': !splitCharts }" @click="splitCharts = false">Ensemble</button>
        <button class="chip" :class="{ 'chip-accent':  splitCharts }" @click="splitCharts = true">Séparés</button>
      </div>
    </div>

    <div v-if="!splitCharts" ref="plotDiv"     class="chart-box"></div>
    <div v-else              class="chart-split">
      <div ref="plotTempDiv" class="chart-box-half"></div>
      <div ref="plotPresDiv" class="chart-box-half"></div>
    </div>

  </div>
</template>

<script setup>
import { ref, watch, onMounted, onBeforeUnmount, nextTick } from 'vue'
import Plotly from 'plotly.js-dist-min'

const props = defineProps({
  temperature: { type: Array, default: () => [] },
  humidity:    { type: Array, default: () => [] },
  timestamps:  { type: Array, default: () => [] }
})

const showTemp    = ref(true)
const showPres    = ref(true)
const splitCharts = ref(true)
const chartType   = ref('lines+markers')

const chartTypes = [
  { label: 'Courbe', value: 'lines+markers' },
  { label: 'Points', value: 'markers' },
  { label: 'Barres', value: 'bar' },
  { label: 'Aires',  value: 'lines' },
]

const plotDiv     = ref(null)
const plotTempDiv = ref(null)
const plotPresDiv = ref(null)

let initMain = false
let initTemp = false
let initPres = false

function isDark()     { return document.documentElement.getAttribute('data-theme') !== 'light' }
function gridColor()  { return isDark() ? '#21262d' : '#e2e8f0' }
function axisColor()  { return isDark() ? '#8b949e' : '#475569' }
function hoverBg()    { return isDark() ? '#1e293b' : '#f8fafc' }
function hoverBdr()   { return isDark() ? '#334155' : '#cbd5e1' }
function hoverText()  { return isDark() ? '#f1f5f9' : '#0f172a' }

// ✅ Labels : seulement heure:minute, max 6 ticks
function makeLabels(arr) {
  if (props.timestamps && props.timestamps.length > 0) {
    return props.timestamps.slice(0, arr.length).map(t => {
      // Garder seulement HH:MM (pas les secondes)
      const parts = t.split(':')
      return parts.length >= 2 ? `${parts[0]}:${parts[1]}` : t
    })
  }
  return Array.from({ length: arr.length }, (_, i) => i + 1)
}

function baseLayout(extra = {}) {
  return {
    paper_bgcolor: 'transparent',
    plot_bgcolor:  'transparent',
    autosize: true,
    margin: { t: 10, r: 60, b: 40, l: 55 },
    showlegend: false,
    font: { family: 'Segoe UI, system-ui, sans-serif' },
    hovermode: 'x unified',
    hoverlabel: {
      bgcolor: hoverBg(), bordercolor: hoverBdr(),
      font: { color: hoverText(), size: 11 }
    },
    ...extra
  }
}

// ✅ Axe X propre — max 6 ticks, pas de rotation
function xAxisConfig(title = '') {
  return {
    color: axisColor(),
    gridcolor: gridColor(),
    tickfont: { color: axisColor(), size: 10 },
    title: title ? { text: title, font: { color: axisColor(), size: 11 } } : undefined,
    nticks: 6,
    fixedrange: false,
    showgrid: true,
  }
}

function getLayoutMain() {
  return baseLayout({
    xaxis: xAxisConfig(),
    yaxis: {
      title: { text: '°C', font: { color: '#38bdf8', size: 11 } },
      color: '#38bdf8', gridcolor: gridColor(),
      tickfont: { color: '#38bdf8', size: 10 },
      range: [0, 50], side: 'left', fixedrange: false
    },
    yaxis2: {
      title: { text: 'hPa', font: { color: '#34d399', size: 11 } },
      color: '#34d399', tickfont: { color: '#34d399', size: 10 },
      range: [950, 1050], overlaying: 'y', side: 'right',
      showgrid: false, fixedrange: false
    },
    legend: {
      orientation: 'h', x: 0.5, xanchor: 'center', y: 1.08,
      font: { color: axisColor(), size: 11 }
    },
    showlegend: true
  })
}

function getLayoutTemp() {
  return baseLayout({
    margin: { t: 10, r: 20, b: 40, l: 50 },
    xaxis: xAxisConfig(),
    yaxis: {
      title: { text: '°C', font: { color: '#38bdf8', size: 11 } },
      color: '#38bdf8', gridcolor: gridColor(),
      tickfont: { color: '#38bdf8', size: 10 },
      range: [0, 50], fixedrange: false
    },
    annotations: [{
      text: 'Température (°C)',
      xref: 'paper', yref: 'paper',
      x: 0.5, y: 1.02, xanchor: 'center', yanchor: 'bottom',
      showarrow: false,
      font: { color: '#38bdf8', size: 12 }
    }]
  })
}

function getLayoutPres() {
  return baseLayout({
    margin: { t: 10, r: 20, b: 40, l: 50 },
    xaxis: xAxisConfig(),
    yaxis: {
      title: { text: 'hPa', font: { color: '#34d399', size: 11 } },
      color: '#34d399', gridcolor: gridColor(),
      tickfont: { color: '#34d399', size: 10 },
      range: [950, 1050], fixedrange: false
    },
    annotations: [{
      text: 'Pression (hPa)',
      xref: 'paper', yref: 'paper',
      x: 0.5, y: 1.02, xanchor: 'center', yanchor: 'bottom',
      showarrow: false,
      font: { color: '#34d399', size: 12 }
    }]
  })
}

function buildTrace(x, y, name, color, yaxis, unit) {
  const isBar  = chartType.value === 'bar'
  const isArea = chartType.value === 'lines'
  return {
    x, y, name,
    type: isBar ? 'bar' : 'scatter',
    mode: isBar ? undefined : chartType.value,
    fill: isArea ? 'tozeroy' : undefined,
    fillcolor: isArea ? `${color}18` : undefined,
    line: !isBar ? { color, width: 2, shape: 'spline' } : undefined,
    marker: { color, size: 4 },
    yaxis,
    hovertemplate: `<b>${name}</b>: %{y:.1f} ${unit}<extra></extra>`
  }
}

function traceTemp(yaxis = 'y')  { return buildTrace(makeLabels(props.temperature), [...props.temperature], 'Température', '#38bdf8', yaxis, '°C') }
function tracePres(yaxis = 'y2') { return buildTrace(makeLabels(props.humidity),    [...props.humidity],    'Pression',    '#34d399', yaxis, 'hPa') }

function getTracesMain() {
  const t = []
  if (showTemp.value) t.push(traceTemp('y'))
  if (showPres.value) t.push(tracePres('y2'))
  return t
}

const plotConfig = {
  responsive: true, autosize: true, displaylogo: false,
  modeBarButtonsToRemove: ['select2d', 'lasso2d', 'autoScale2d'],
  toImageButtonOptions: { format: 'png', filename: 'iot-graph' }
}

async function drawMain() {
  if (!plotDiv.value) return
  if (!initMain) { await Plotly.newPlot(plotDiv.value, getTracesMain(), getLayoutMain(), plotConfig); initMain = true }
  else Plotly.react(plotDiv.value, getTracesMain(), getLayoutMain())
}

async function drawSplit() {
  if (plotTempDiv.value && showTemp.value) {
    if (!initTemp) { await Plotly.newPlot(plotTempDiv.value, [traceTemp('y')], getLayoutTemp(), plotConfig); initTemp = true }
    else Plotly.react(plotTempDiv.value, [traceTemp('y')], getLayoutTemp())
  }
  if (plotPresDiv.value && showPres.value) {
    if (!initPres) { await Plotly.newPlot(plotPresDiv.value, [tracePres('y')], getLayoutPres(), plotConfig); initPres = true }
    else Plotly.react(plotPresDiv.value, [tracePres('y')], getLayoutPres())
  }
}

async function update() {
  await nextTick()
  splitCharts.value ? await drawSplit() : await drawMain()
}

onMounted(async () => { await nextTick(); await update() })

watch([
  () => props.temperature.length,
  () => props.humidity.length,
  () => props.timestamps.length,
  showTemp, showPres, chartType, splitCharts
], update)

onBeforeUnmount(() => {
  if (plotDiv.value)     Plotly.purge(plotDiv.value)
  if (plotTempDiv.value) Plotly.purge(plotTempDiv.value)
  if (plotPresDiv.value) Plotly.purge(plotPresDiv.value)
})
</script>

<style scoped>
.chart-wrapper {
  display: flex;
  flex-direction: column;
  gap: 8px;
  width: 100%;
  height: 100%;
}

.chart-controls {
  display: flex;
  flex-wrap: wrap;
  gap: 6px;
  align-items: center;
  flex-shrink: 0;
}

.ctrl-group { display: flex; align-items: center; gap: 4px; }

.ctrl-label { font-size: 10px; color: var(--text-muted); white-space: nowrap; }

.chip {
  background: var(--bg3); border: 1px solid var(--border);
  color: var(--text-muted); font-size: 10px;
  padding: 2px 8px; border-radius: 20px; cursor: pointer; transition: all 0.2s;
}

.chip:hover     { border-color: var(--primary); color: var(--primary); }
.chip-accent    { background: rgba(14,165,233,0.15); border-color: var(--accent); color: var(--accent); }
.chip-green     { background: rgba(52,211,153,0.15); border-color: var(--secondary); color: var(--secondary); }
.chip-blue      { background: rgba(56,189,248,0.15); border-color: var(--primary); color: var(--primary); }

/* ✅ Graphiques prennent tout l'espace restant */
.chart-box {
  width: 100%;
  flex: 1;
  min-height: 160px;
}

.chart-split {
  display: flex;
  gap: 8px;
  flex: 1;
  min-height: 160px;
}

.chart-box-half {
  flex: 1;
  min-width: 0;
  height: 100%;
}
</style>