import React from 'react';
import WaveAnimation from '../WaveAnimation';
import './LandingPage.css';

interface LandingPageProps {
  onBegin: () => void;
}

const LandingPage: React.FC<LandingPageProps> = ({ onBegin }) => {
  return (
    <div className="landing-page">
      <div className="landing-content">
        <WaveAnimation title="Marble" height={250} />
        <p className="landing-quote">Toy Stock Ingestion Backtesting, & Visualizing</p>
        <button className="landing-button" onClick={onBegin}>
          Begin
        </button>
      </div>
    </div>
  );
};

export default LandingPage;
