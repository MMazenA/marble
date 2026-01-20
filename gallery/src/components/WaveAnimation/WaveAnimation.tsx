import React, { useEffect, useRef, useState } from 'react';
import * as echarts from 'echarts';
import './WaveAnimation.css';

interface WaveAnimationProps {
  title?: string;
  spacing?: number;
  minSize?: number;
  maxSize?: number;
  height?: number;
}

// Simplex and perlin noise helper from https://github.com/josephg/noisejs
function getNoiseHelper() {
  class Grad {
    x: number;
    y: number;
    z: number;
    constructor(x: number, y: number, z: number) {
      this.x = x;
      this.y = y;
      this.z = z;
    }
    dot2(x: number, y: number) {
      return this.x * x + this.y * y;
    }
  }

  const grad3 = [
    new Grad(1, 1, 0),
    new Grad(-1, 1, 0),
    new Grad(1, -1, 0),
    new Grad(-1, -1, 0),
    new Grad(1, 0, 1),
    new Grad(-1, 0, 1),
    new Grad(1, 0, -1),
    new Grad(-1, 0, -1),
    new Grad(0, 1, 1),
    new Grad(0, -1, 1),
    new Grad(0, 1, -1),
    new Grad(0, -1, -1),
  ];

  const p = [
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
    36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
    234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
    88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
    230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
    1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130,
    116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
    124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227,
    47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44,
    154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98,
    108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34,
    242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
    239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121,
    50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243,
    141, 128, 195, 78, 66, 215, 61, 156, 180,
  ];

  const perm = new Array(512);
  const gradP = new Array<Grad>(512);

  function seed(seedVal: number) {
    if (seedVal > 0 && seedVal < 1) {
      seedVal *= 65536;
    }
    seedVal = Math.floor(seedVal);
    if (seedVal < 256) {
      seedVal |= seedVal << 8;
    }
    for (let i = 0; i < 256; i++) {
      let v;
      if (i & 1) {
        v = p[i] ^ (seedVal & 255);
      } else {
        v = p[i] ^ ((seedVal >> 8) & 255);
      }
      perm[i] = perm[i + 256] = v;
      gradP[i] = gradP[i + 256] = grad3[v % 12];
    }
  }

  seed(0);

  function fade(t: number) {
    return t * t * t * (t * (t * 6 - 15) + 10);
  }

  function lerp(a: number, b: number, t: number) {
    return (1 - t) * a + t * b;
  }

  function perlin2(x: number, y: number) {
    let X = Math.floor(x);
    let Y = Math.floor(y);
    x = x - X;
    y = y - Y;
    X = X & 255;
    Y = Y & 255;
    const n00 = gradP[X + perm[Y]].dot2(x, y);
    const n01 = gradP[X + perm[Y + 1]].dot2(x, y - 1);
    const n10 = gradP[X + 1 + perm[Y]].dot2(x - 1, y);
    const n11 = gradP[X + 1 + perm[Y + 1]].dot2(x - 1, y - 1);
    const u = fade(x);
    return lerp(lerp(n00, n10, u), lerp(n01, n11, u), fade(y));
  }

  return { seed, perlin2 };
}

const WaveAnimation: React.FC<WaveAnimationProps> = ({
  title = 'Marble',
  spacing = 5,
  minSize = 1,
  maxSize = 4,
  height = 300,
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const chartRef = useRef<HTMLDivElement>(null);
  const chartInstance = useRef<echarts.ECharts | null>(null);
  const [maskUrl, setMaskUrl] = useState<string>('');

  useEffect(() => {
    if (!containerRef.current) return;

    const updateMask = () => {
      const container = containerRef.current;
      if (!container) return;

      const width = container.offsetWidth;
      const containerHeight = container.offsetHeight;

      // Create SVG mask with text
      const svg = `
        <svg xmlns="http://www.w3.org/2000/svg" width="${width}" height="${containerHeight}">
          <text
            x="50%"
            y="50%"
            text-anchor="middle"
            dominant-baseline="central"
            fill="white"
            font-size="160"
            font-weight="700"
            font-family="Gambarino, sans-serif"
          >${title}</text>
        </svg>
      `;
      const encoded = encodeURIComponent(svg);
      setMaskUrl(`url("data:image/svg+xml,${encoded}")`);
    };

    updateMask();
    window.addEventListener('resize', updateMask);
    return () => window.removeEventListener('resize', updateMask);
  }, [title, height]);

  useEffect(() => {
    if (!chartRef.current) return;

    const noise = getNoiseHelper();
    noise.seed(Math.random());

    const config = {
      frequency: 500,
      offsetX: 20,
      offsetY: 100,
      minSize,
      maxSize,
      duration: 5000,
      colorBright: '#e4e4e7',
      colorDark: '#1a1a24',
      backgroundColor: 'transparent',
    };

    chartInstance.current = echarts.init(chartRef.current);

    const createElements = () => {
      const elements: any[] = [];
      const chart = chartInstance.current!;
      const width = chart.getWidth();
      const chartHeight = chart.getHeight();

      for (let x = spacing / 2; x < width; x += spacing) {
        for (let y = spacing / 2; y < chartHeight; y += spacing) {
          const rand = noise.perlin2(
            x / config.frequency + config.offsetX,
            y / config.frequency + config.offsetY
          );
          // Spread delays across the full duration for large sweeping waves
          const delay = rand * config.duration;

          elements.push({
            type: 'circle',
            x,
            y,
            silent: true,
            style: {
              fill: config.colorBright,
            },
            shape: {
              r: config.maxSize,
            },
            keyframeAnimation: {
              duration: config.duration,
              loop: true,
              delay,
              keyframes: [
                {
                  percent: 0.5,
                  easing: 'sinusoidalInOut',
                  style: {
                    fill: config.colorDark,
                  },
                  scaleX: config.minSize / config.maxSize,
                  scaleY: config.minSize / config.maxSize,
                },
                {
                  percent: 1,
                  easing: 'sinusoidalInOut',
                  style: {
                    fill: config.colorBright,
                  },
                  scaleX: 1,
                  scaleY: 1,
                },
              ],
            },
          });
        }
      }
      return elements;
    };

    const option: echarts.EChartsOption = {
      backgroundColor: config.backgroundColor,
      graphic: {
        elements: createElements(),
      },
    };

    chartInstance.current.setOption(option);

    const handleResize = () => {
      if (chartInstance.current) {
        chartInstance.current.resize();
        chartInstance.current.setOption({
          graphic: {
            elements: createElements(),
          },
        });
      }
    };

    window.addEventListener('resize', handleResize);

    return () => {
      window.removeEventListener('resize', handleResize);
      chartInstance.current?.dispose();
    };
  }, [spacing, minSize, maxSize]);

  return (
    <div className="wave-animation-container" ref={containerRef} style={{ height }}>
      <div
        className="wave-animation-chart"
        ref={chartRef}
        style={{
          WebkitMaskImage: maskUrl,
          maskImage: maskUrl,
          WebkitMaskSize: '100% 100%',
          maskSize: '100% 100%',
          WebkitMaskRepeat: 'no-repeat',
          maskRepeat: 'no-repeat',
        }}
      />
    </div>
  );
};

export default WaveAnimation;
