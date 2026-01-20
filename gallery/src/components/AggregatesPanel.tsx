import React, { useState } from 'react';
import { AggregatesServiceClient } from '@proto/AggregatesServiceClientPb';
import { AggregateBar, AggregatesRequest, timespan_options } from '@proto/aggregates_pb';
import CandlestickChart from './CandlestickChart/CandlestickChart';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import { AdapterDayjs } from '@mui/x-date-pickers/AdapterDayjs';
import { DatePicker } from '@mui/x-date-pickers/DatePicker';
import { ThemeProvider, createTheme } from '@mui/material/styles';
import dayjs, { Dayjs } from 'dayjs';

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

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
    primary: { main: '#3b82f6' },
    background: {
      default: '#0d0d14',
      paper: '#1a1a24',
    },
  },
  components: {
    MuiTextField: {
      styleOverrides: {
        root: {
          '& .MuiOutlinedInput-root': {
            backgroundColor: '#12121a',
            borderRadius: '6px',
            '& fieldset': { borderColor: '#2d2d3a' },
            '&:hover fieldset': { borderColor: '#3b82f6' },
            '&.Mui-focused fieldset': { borderColor: '#3b82f6' },
          },
          '& .MuiInputBase-input': {
            color: '#e4e4e7',
            padding: '10px 14px',
          },
        },
      },
    },
  },
});

export default function AggregatesPanel() {
  const [ticker, setTicker] = useState('AAPL');
  const [fromDate, setFromDate] = useState<Dayjs | null>(() => dayjs().subtract(1, 'month'));
  const [toDate, setToDate] = useState<Dayjs | null>(() => dayjs());
  const [timespan, setTimespan] = useState<timespan_options>(timespan_options.DAY);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [resultCount, setResultCount] = useState<number | null>(null);
  const [barsResult, setBarsResult] = useState<AggregateBar.AsObject[] | null>(null);

  const handleFetch = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!fromDate || !toDate) {
      setError('Please select both dates');
      return;
    }
    setIsLoading(true);
    setError(null);
    setResultCount(null);

    const request = new AggregatesRequest();
    request.setTicker(ticker.toUpperCase());
    request.setFromDate(fromDate.format('YYYY-MM-DD'));
    request.setToDate(toDate.format('YYYY-MM-DD'));
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


  return (
    <ThemeProvider theme={darkTheme}>
      <LocalizationProvider dateAdapter={AdapterDayjs}>
        <div>
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
                  <DatePicker
                    value={fromDate}
                    onChange={(newValue) => setFromDate(newValue)}
                    maxDate={toDate ?? undefined}
                    slotProps={{
                      textField: { size: 'small', fullWidth: true },
                    }}
                  />
                </div>

                <div>
                  <label className="field-label">To</label>
                  <DatePicker
                    value={toDate}
                    onChange={(newValue) => setToDate(newValue)}
                    minDate={fromDate ?? undefined}
                    maxDate={dayjs()}
                    slotProps={{
                      textField: { size: 'small', fullWidth: true },
                    }}
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

            <div className="chart-column">
              {barsResult && <CandlestickChart data={barsResult} ticker={ticker} />}
            </div>
          </div>
        </div>
      </LocalizationProvider>
    </ThemeProvider>
  );
}
