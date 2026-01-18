import React, { useState } from 'react';
import { AggregatesServiceClient } from '@proto/AggregatesServiceClientPb';
import { AggregateBar, AggregatesRequest, timespan_options } from '@proto/aggregates_pb';

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

const client = new AggregatesServiceClient('');

export default function AggregatesPanel() {
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
  const [barsResult, setBarsResult] = useState<AggregateBar.AsObject[] | null>(null);

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
      const bars = response.toObject().aggregateBarsList;
      setBarsResult(bars);
      setResultCount(bars.length);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch data');
    } finally {
      setIsLoading(false);
    }
  };

  const listBars = barsResult?.map((bar, idx) => (
    <li key={idx} className="bars-list__row">
      <span>{bar.high}</span>
      <span>{bar.low}</span>
    </li>
  ));

  return (
    <div>
      <div className="center main-title">Marble</div>
      <h3>Aggregates Endpoint</h3>

      <div className="layout-grid">
        <div className="form-column">
          <form onSubmit={handleFetch} className="aggregate-form">
            <div>
              <label className="field-label">Ticker</label>
              <input
                type="text"
                value={ticker}
                onChange={(e) => setTicker(e.target.value.toUpperCase())}
                className="field-input"
              />
            </div>

            <div>
              <label className="field-label">From</label>
              <input
                type="date"
                value={fromDate}
                onChange={(e) => setFromDate(e.target.value)}
                className="field-input field-input--date"
              />
            </div>

            <div>
              <label className="field-label">To</label>
              <input
                type="date"
                value={toDate}
                onChange={(e) => setToDate(e.target.value)}
                className="field-input field-input--date"
              />
            </div>

            <div>
              <label className="field-label">Timespan</label>
              <select
                value={timespan}
                onChange={(e) => setTimespan(Number(e.target.value))}
                className="field-input"
              >
                {Object.entries(timespanLabels).map(([value, label]) => (
                  <option key={value} value={value}>
                    {label}
                  </option>
                ))}
              </select>
            </div>

            <button type="submit" disabled={isLoading}>
              {isLoading ? 'Loading...' : 'Fetch Data'}
            </button>
          </form>
          {error && (
            <p className="error" style={{ marginTop: '1rem' }}>
              {error}
            </p>
          )}
          {resultCount !== null && (
            <p className="success" style={{ marginTop: '1rem' }}>
              Fetched {resultCount} bars.
            </p>
          )}
        </div>

        <div className="results-column">
          <ul className="bars-list">
            <li className="bars-list__header">
              <span>High</span>
              <span>Low</span>
            </li>
            {listBars}
          </ul>
        </div>
      </div>
    </div>
  );
}
