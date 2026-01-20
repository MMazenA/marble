# Gallery

Results & Strategy Visualizer


## Dependencies
- PNPM ^10.28.0

## Running
- Make sure envoy is running from docker
- `cd gallery`
- `pnpm i`
- `pnpm proto:gen`
- `pnpm start`

## Charting options
- https://echarts.apache.org/examples/en/editor.html?c=candlestick-simple&lang=ts
- https://echarts.apache.org/examples/en/index.html#chart-type-candlestick
- https://recharts.github.io/en-US/examples/Candlestick/

- https://echarts.apache.org/examples/en/editor.html?c=heatmap-large
  - X -> time
  - Y -> Single value parametrized function
         backtesting engine should support dynamic creation of these 
