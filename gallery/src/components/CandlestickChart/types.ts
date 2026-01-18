import { AggregateBar } from '@proto/aggregates_pb';

export interface Trade {
  timestamp: number;
  price: number;
  side: 'buy' | 'sell';
  quantity?: number;
}

export interface CandlestickChartProps {
  data: AggregateBar.AsObject[];
  ticker: string;
  trades?: Trade[];
}

export interface TradeZone {
  xAxis: string;
  yAxis: number;
}
