import * as echarts from 'echarts';
import { AggregateBar } from '@proto/aggregates_pb';
import { Trade, TradeZone } from './types';
import { chartColors, chartTheme } from './theme';
import { formatDate, computeTradeZones } from './utils';

interface ChartData {
  data: AggregateBar.AsObject[];
  trades: Trade[];
  ticker: string;
}

export function buildChartOptions({ data, trades, ticker }: ChartData): echarts.EChartsOption {
  const dates = data.map((bar) => formatDate(bar.t));
  const timestampToDateStr = (t: number) => formatDate(t);

  const candlestickData = data.map((bar) => [bar.open, bar.close, bar.low, bar.high]);

  const volumeData = data.map((bar) => ({
    value: bar.volume,
    itemStyle: {
      color: bar.close >= bar.open ? chartColors.bullishFaded : chartColors.bearishFaded,
    },
  }));

  const buyTrades = trades
    .filter((t) => t.side === 'buy')
    .map((t) => [timestampToDateStr(t.timestamp), t.price]);

  const sellTrades = trades
    .filter((t) => t.side === 'sell')
    .map((t) => [timestampToDateStr(t.timestamp), t.price]);

  const tradeZones = computeTradeZones(trades, timestampToDateStr);

  const tradeZoneAreas = tradeZones.map((zone) => {
    const [start, end] = zone;
    const isProfit = end.yAxis > start.yAxis;
    return {
      itemStyle: {
        color: isProfit ? chartColors.bullishZone : chartColors.bearishZone,
        borderColor: isProfit ? chartColors.bullish : chartColors.bearish,
        borderWidth: 1,
      },
      data: zone as [TradeZone, TradeZone],
    };
  });

  return {
    backgroundColor: 'transparent',
    title: {
      text: ticker,
      left: 'center',
      textStyle: chartTheme.title.textStyle,
    },
    legend: {
      data: ['Candlestick', 'Buy', 'Sell', 'Volume'],
      top: 30,
      textStyle: chartTheme.legend.textStyle,
    },
    tooltip: {
      trigger: 'axis',
      axisPointer: { type: 'cross' },
      ...chartTheme.tooltip,
      formatter: (params: unknown) => {
        const p = params as echarts.DefaultLabelFormatterCallbackParams[];
        if (!p || p.length === 0) return '';

        const dataIndex = p[0]?.dataIndex;
        if (dataIndex === undefined || dataIndex < 0 || dataIndex >= data.length) return '';

        const bar = data[dataIndex];
        const dateStr = formatDate(bar.t, true);

        let html = `
          <div style="font-size: 12px;">
            <div style="margin-bottom: 4px; font-weight: bold;">${dateStr}</div>
            <div>Open: ${bar.open.toFixed(2)}</div>
            <div>Close: ${bar.close.toFixed(2)}</div>
            <div>High: ${bar.high.toFixed(2)}</div>
            <div>Low: ${bar.low.toFixed(2)}</div>
            <div>Volume: ${bar.volume.toLocaleString()}</div>
          </div>
        `;

        const buyPoint = p.find((item) => item.seriesName === 'Buy');
        const sellPoint = p.find((item) => item.seriesName === 'Sell');
        if (buyPoint && Array.isArray(buyPoint.data)) {
          html += `<div style="color: ${chartColors.bullish}; margin-top: 4px;">BUY @ ${(buyPoint.data[1] as number).toFixed(2)}</div>`;
        }
        if (sellPoint && Array.isArray(sellPoint.data)) {
          html += `<div style="color: ${chartColors.bearish}; margin-top: 4px;">SELL @ ${(sellPoint.data[1] as number).toFixed(2)}</div>`;
        }

        return html;
      },
    },
    axisPointer: {
      link: [{ xAxisIndex: 'all' }],
    },
    grid: [
      { left: '10%', right: '10%', top: '12%', height: '50%' },
      { left: '10%', right: '10%', top: '68%', height: '12%' },
    ],
    xAxis: [
      {
        type: 'category',
        data: dates,
        axisLine: chartTheme.axisLine,
        axisLabel: {
          ...chartTheme.axisLabel,
          rotate: 0,
          hideOverlap: true,
        },
        axisTick: { alignWithLabel: true },
        gridIndex: 0,
      },
      {
        type: 'category',
        data: dates,
        axisLine: chartTheme.axisLine,
        axisLabel: { show: false },
        gridIndex: 1,
      },
    ],
    yAxis: [
      {
        type: 'value',
        scale: true,
        splitLine: chartTheme.splitLine,
        axisLine: chartTheme.axisLine,
        axisLabel: chartTheme.axisLabel,
        gridIndex: 0,
      },
      {
        type: 'value',
        scale: true,
        splitLine: { show: false },
        axisLine: chartTheme.axisLine,
        axisLabel: { show: false },
        gridIndex: 1,
      },
    ],
    dataZoom: [
      {
        type: 'inside',
        xAxisIndex: [0, 1],
        start: 0,
        end: 100,
        zoomOnMouseWheel: true,
        moveOnMouseMove: true,
        moveOnMouseWheel: false,
        preventDefaultMouseMove: false,
        throttle: 50,
      },
      {
        type: 'slider',
        xAxisIndex: [0, 1],
        start: 0,
        end: 100,
        height: 24,
        bottom: 10,
        borderColor: chartColors.border,
        backgroundColor: chartColors.background,
        fillerColor: chartColors.primaryFaded,
        handleStyle: {
          color: chartColors.primary,
          borderColor: chartColors.primary,
        },
        moveHandleStyle: {
          color: chartColors.primary,
        },
        textStyle: { color: chartColors.textMuted },
        labelFormatter: (_: number, valueStr: string) => valueStr,
        brushSelect: true,
        brushStyle: {
          color: chartColors.primaryLight,
        },
      },
    ],
    toolbox: {
      show: true,
      right: '10%',
      top: 0,
      feature: {
        dataZoom: {
          yAxisIndex: false,
          title: { zoom: 'Zoom', back: 'Undo Zoom' },
        },
        restore: { title: 'Reset' },
      },
      iconStyle: {
        borderColor: chartColors.textMuted,
      },
      emphasis: {
        iconStyle: {
          borderColor: chartColors.primary,
        },
      },
    },
    series: [
      {
        name: 'Candlestick',
        type: 'candlestick',
        data: candlestickData,
        xAxisIndex: 0,
        yAxisIndex: 0,
        itemStyle: {
          color: chartColors.bullish,
          color0: chartColors.bearish,
          borderColor: chartColors.bullish,
          borderColor0: chartColors.bearish,
        },
      },
      ...tradeZoneAreas.map((zone, idx) => ({
        name: `Zone ${idx + 1}`,
        type: 'line' as const,
        xAxisIndex: 0,
        yAxisIndex: 0,
        data: [],
        markArea: {
          silent: true,
          itemStyle: zone.itemStyle,
          data: [zone.data],
        },
      })),
      {
        name: 'Volume',
        type: 'bar',
        data: volumeData,
        xAxisIndex: 1,
        yAxisIndex: 1,
      },
      {
        name: 'Buy',
        type: 'scatter',
        data: buyTrades,
        xAxisIndex: 0,
        yAxisIndex: 0,
        symbol: 'triangle',
        symbolSize: 14,
        itemStyle: { color: chartColors.bullish },
        z: 10,
      },
      {
        name: 'Sell',
        type: 'scatter',
        data: sellTrades,
        xAxisIndex: 0,
        yAxisIndex: 0,
        symbol: 'triangle',
        symbolRotate: 180,
        symbolSize: 14,
        itemStyle: { color: chartColors.bearish },
        z: 10,
      },
    ],
  };
}
