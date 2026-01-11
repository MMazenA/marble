import React, { useState } from 'react';
import { AggregatesServiceClient } from './proto/AggregatesServiceClientPb';
import { AggregatesRequest, timespan_options } from './proto/aggregates_pb';

const client = new AggregatesServiceClient('');

const timespanLabels: Record<number, string> = {
  [timespan_options.SECOND]: 'Second',
  [timespan_options.MINUTE]: 'Minute',
  [timespan_options.HOUR]: 'Hour',
  [timespan_options.DAY]: 'Day',
  [timespan_options.WEEK]: 'Week',
  [timespan_options.MONTH]: 'Month',
  [timespan_options.QUARTER]: 'Quarter',
  [timespan_options.YEAR]: 'Year',
};

function App() {
  const [ticker, setTicker] = useState('AAPL');
  const [fromDate, setFromDate] = useState(() => {
    const date = new Date();
    date.setMonth(date.getMonth() - 1);
    return date.toISOString().split('T')[0];
  });
  const [toDate, setToDate] = useState(() => new Date().toISOString().split('T')[0]);
  const [timespan, setTimespan] = useState<timespan_options>(timespan_options.DAY);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [resultCount, setResultCount] = useState<number | null>(null);

  const handleFetch = async (e: React.FormEvent) => {
    e.preventDefault();
    setIsLoading(true);
    setError(null);
    setResultCount(null);

    const request = new AggregatesRequest();
    request.setTicker(ticker.toUpperCase());
    request.setFromDate(fromDate);
    request.setToDate(toDate);
    request.setTimeSpan(timespan);

    try {
      const response = await client.getAggregate(request, null);
      const bars = response.getAggregateBarsList();

      console.log('Fetched aggregates:', bars.map(bar => ({
        time: bar.getT(),
        open: bar.getOpen(),
        high: bar.getHigh(),
        low: bar.getLow(),
        close: bar.getClose(),
        volume: bar.getVolume(),
      })));

      setResultCount(bars.length);
    } catch (err) {
      console.error('Failed to fetch:', err);
      setError(err instanceof Error ? err.message : 'Failed to fetch data');
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div>
      <h1>Marble - Fetch Aggregates</h1>

      <form onSubmit={handleFetch} style={{ display: 'flex', flexDirection: 'column', gap: '1rem', maxWidth: '300px' }}>
        <div>
          <label style={{ display: 'block', marginBottom: '0.25rem' }}>Ticker</label>
          <input
            type="text"
            value={ticker}
            onChange={(e) => setTicker(e.target.value.toUpperCase())}
            style={{ width: '100%' }}
          />
        </div>

        <div>
          <label style={{ display: 'block', marginBottom: '0.25rem' }}>From</label>
          <input
            type="date"
            value={fromDate}
            onChange={(e) => setFromDate(e.target.value)}
            style={{ width: '100%', colorScheme: 'dark' }}
          />
        </div>

        <div>
          <label style={{ display: 'block', marginBottom: '0.25rem' }}>To</label>
          <input
            type="date"
            value={toDate}
            onChange={(e) => setToDate(e.target.value)}
            style={{ width: '100%', colorScheme: 'dark' }}
          />
        </div>

        <div>
          <label style={{ display: 'block', marginBottom: '0.25rem' }}>Timespan</label>
          <select
            value={timespan}
            onChange={(e) => setTimespan(Number(e.target.value))}
            style={{ width: '100%' }}
          >
            {Object.entries(timespanLabels).map(([value, label]) => (
              <option key={value} value={value}>{label}</option>
            ))}
          </select>
        </div>

        <button type="submit" disabled={isLoading}>
          {isLoading ? 'Loading...' : 'Fetch Data'}
        </button>
      </form>

      {error && <p className="error" style={{ marginTop: '1rem' }}>{error}</p>}
      {resultCount !== null && (
        <p className="success" style={{ marginTop: '1rem' }}>
          Fetched {resultCount} bars.
        </p>
      )}
    </div>
  );
}

export default App;
