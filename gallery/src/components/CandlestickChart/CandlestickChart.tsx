import { useEffect, useRef } from 'react';
import * as echarts from 'echarts';
import { CandlestickChartProps } from './types';
import { generateDummyTrades } from './utils';
import { buildChartOptions } from './chartConfig';

const USE_DUMMY_TRADES = true;

export default function CandlestickChart({ data, ticker, trades }: CandlestickChartProps) {
  const chartRef = useRef<HTMLDivElement>(null);
  const chartInstance = useRef<echarts.ECharts | null>(null);

  useEffect(() => {
    if (!chartRef.current) {
      return;
    }

    chartInstance.current = echarts.init(chartRef.current);

    const handleResize = () => chartInstance.current?.resize();
    window.addEventListener('resize', handleResize);

    return () => {
      window.removeEventListener('resize', handleResize);
      chartInstance.current?.dispose();
    };
  }, []);

  useEffect(() => {
    if (!chartInstance.current || data.length === 0) {
      return;
    }

    const activeTrades = trades ?? (USE_DUMMY_TRADES ? generateDummyTrades(data) : []);
    const options = buildChartOptions({ data, trades: activeTrades, ticker });
    chartInstance.current.setOption(options);
  }, [data, ticker, trades]);

  if (data.length === 0) {
    return <div className="chart-empty">No data to display</div>;
  }

  return <div ref={chartRef} className="chart-container" />;
}
