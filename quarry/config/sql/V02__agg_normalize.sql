CREATE TABLE IF NOT EXISTS dim_tickers (
  ticker_id VARCHAR(20) PRIMARY KEY,
  source VARCHAR(32) NOT NULL DEFAULT 'massive',
  display_name VARCHAR(128),
  last_ingested_request_id VARCHAR(100),
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS fact_aggregate_bars (
  ticker_id VARCHAR(20) NOT NULL REFERENCES dim_tickers (ticker_id),
  bar_timestamp BIGINT NOT NULL,
  open DOUBLE PRECISION NOT NULL,
  close DOUBLE PRECISION NOT NULL,
  high DOUBLE PRECISION NOT NULL,
  low DOUBLE PRECISION NOT NULL,
  transactions BIGINT NOT NULL,
  is_otc BOOLEAN NOT NULL,
  volume DOUBLE PRECISION NOT NULL,
  volume_weighted DOUBLE PRECISION NOT NULL,
  request_id VARCHAR(100),
  created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
  PRIMARY KEY (ticker_id, bar_timestamp)
);

DO $$
BEGIN
  IF NOT EXISTS (
    SELECT 1
    FROM information_schema.table_constraints
    WHERE constraint_name = 'fk_stg_agg_meta_ticker'
      AND table_name = 'stg_aggregates_meta'
  ) THEN
    ALTER TABLE stg_aggregates_meta
    ADD CONSTRAINT fk_stg_agg_meta_ticker
    FOREIGN KEY (ticker_id) REFERENCES dim_tickers (ticker_id);
  END IF;
END;
$$;

CREATE OR REPLACE FUNCTION normalize_aggregate_stage(
  p_ticker_id VARCHAR,
  p_request_id VARCHAR,
  p_display_name VARCHAR,
  p_source VARCHAR
) RETURNS VOID AS $$
BEGIN
  INSERT INTO dim_tickers (
    ticker_id,
    source,
    display_name,
    last_ingested_request_id
  )
  VALUES (
    p_ticker_id,
    p_source,
    COALESCE(p_display_name, p_ticker_id),
    p_request_id
  )
  ON CONFLICT (ticker_id) DO UPDATE
    SET display_name = COALESCE(EXCLUDED.display_name, dim_tickers.display_name),
        last_ingested_request_id = EXCLUDED.last_ingested_request_id,
        updated_at = NOW();

  INSERT INTO fact_aggregate_bars (
    ticker_id,
    bar_timestamp,
    open,
    close,
    high,
    low,
    transactions,
    is_otc,
    volume,
    volume_weighted,
    request_id
  )
  SELECT
    p_ticker_id,
    t,
    o,
    c,
    h,
    l,
    n,
    otc,
    v,
    vw,
    p_request_id
  FROM stg_aggregates_results
  ON CONFLICT (ticker_id, bar_timestamp) DO UPDATE
  SET open = EXCLUDED.open,
      close = EXCLUDED.close,
      high = EXCLUDED.high,
      low = EXCLUDED.low,
      transactions = EXCLUDED.transactions,
      is_otc = EXCLUDED.is_otc,
      volume = EXCLUDED.volume,
      volume_weighted = EXCLUDED.volume_weighted,
      request_id = EXCLUDED.request_id,
      updated_at = NOW();

  TRUNCATE TABLE stg_aggregates_results;
END;
$$ LANGUAGE plpgsql;
