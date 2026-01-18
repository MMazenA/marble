import { AggregateBar } from '@proto/aggregates_pb';
import { Trade, TradeZone } from './types';

export function formatDate(timestamp: number, includeYear = false): string {
  const date = new Date(timestamp);
  if (includeYear) {
    return date.toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: '2-digit' });
  }
  return date.toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
}

export function generateDummyTrades(bars: AggregateBar.AsObject[]): Trade[] {
  if (bars.length < 10) {
    return [];
  }

  const trades: Trade[] = [];

  // win
  const buy1Idx = Math.floor(bars.length * 0.1);
  const sell1Idx = Math.floor(bars.length * 0.25);
  if (bars[buy1Idx] && bars[sell1Idx]) {
    trades.push({ timestamp: bars[buy1Idx].t, price: bars[buy1Idx].low, side: 'buy' });
    trades.push({ timestamp: bars[sell1Idx].t, price: bars[sell1Idx].high, side: 'sell' });
  }

  // loss
  const buy2Idx = Math.floor(bars.length * 0.35);
  const sell2Idx = Math.floor(bars.length * 0.5);
  if (bars[buy2Idx] && bars[sell2Idx]) {
    trades.push({ timestamp: bars[buy2Idx].t, price: bars[buy2Idx].high, side: 'buy' });
    trades.push({ timestamp: bars[sell2Idx].t, price: bars[sell2Idx].low, side: 'sell' });
  }

  // win
  const buy3Idx = Math.floor(bars.length * 0.6);
  const sell3Idx = Math.floor(bars.length * 0.75);
  if (bars[buy3Idx] && bars[sell3Idx]) {
    trades.push({ timestamp: bars[buy3Idx].t, price: bars[buy3Idx].low, side: 'buy' });
    trades.push({ timestamp: bars[sell3Idx].t, price: bars[sell3Idx].high, side: 'sell' });
  }

  // loss
  const buy4Idx = Math.floor(bars.length * 0.8);
  const sell4Idx = Math.floor(bars.length * 0.95);
  if (bars[buy4Idx] && bars[sell4Idx]) {
    trades.push({ timestamp: bars[buy4Idx].t, price: bars[buy4Idx].high, side: 'buy' });
    trades.push({ timestamp: bars[sell4Idx].t, price: bars[sell4Idx].low, side: 'sell' });
  }

  return trades;
}

export function computeTradeZones(
  trades: Trade[],
  timestampToDateStr: (t: number) => string
): Array<[TradeZone, TradeZone]> {
  const sortedTrades = [...trades].sort((a, b) => a.timestamp - b.timestamp);
  const zones: Array<[TradeZone, TradeZone]> = [];

  let openBuy: Trade | null = null;
  for (const trade of sortedTrades) {
    if (trade.side === 'buy' && !openBuy) {
      openBuy = trade;
    } else if (trade.side === 'sell' && openBuy) {
      zones.push([
        { xAxis: timestampToDateStr(openBuy.timestamp), yAxis: openBuy.price },
        { xAxis: timestampToDateStr(trade.timestamp), yAxis: trade.price },
      ]);
      openBuy = null;
    }
  }

  return zones;
}
