export const chartColors = {
  bullish: '#22c55e',
  bearish: '#ef4444',
  bullishFaded: 'rgba(34, 197, 94, 0.5)',
  bearishFaded: 'rgba(239, 68, 68, 0.5)',
  bullishZone: 'rgba(34, 197, 94, 0.2)',
  bearishZone: 'rgba(239, 68, 68, 0.2)',
  primary: '#3b82f6',
  primaryFaded: 'rgba(59, 130, 246, 0.3)',
  primaryLight: 'rgba(59, 130, 246, 0.15)',
  text: '#e4e4e7',
  textMuted: '#a1a1aa',
  border: '#2d2d3a',
  background: '#12121a',
  tooltip: '#1a1a24',
} as const;

export const chartTheme = {
  title: {
    textStyle: { color: chartColors.text, fontSize: 16 },
  },
  legend: {
    textStyle: { color: chartColors.textMuted },
  },
  axisLine: {
    lineStyle: { color: chartColors.border },
  },
  axisLabel: {
    color: chartColors.textMuted,
  },
  splitLine: {
    lineStyle: { color: chartColors.border },
  },
  tooltip: {
    backgroundColor: chartColors.tooltip,
    borderColor: chartColors.border,
    textStyle: { color: chartColors.text },
  },
} as const;
