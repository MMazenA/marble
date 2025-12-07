CREATE TABLE stg_aggregates_meta (
  ticker_id VARCHAR(20) NOT NULL,
  request_id VARCHAR(100) NOT NULL
);

CREATE TABLE stg_aggregates_results (
    o      DOUBLE PRECISION NOT NULL,
    c      DOUBLE PRECISION NOT NULL,
    h      DOUBLE PRECISION NOT NULL,
    l      DOUBLE PRECISION NOT NULL,
    n      BIGINT NOT NULL,
    otc    BOOLEAN NOT NULL,
    t      BIGINT NOT NULL,
    v      DOUBLE PRECISION NOT NULL,
    vw     DOUBLE PRECISION NOT NULL,
    PRIMARY KEY (t)
);
