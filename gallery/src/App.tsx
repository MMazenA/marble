import { useState } from 'react';
import LandingPage from '@components/LandingPage';
import AggregatesPanel from '@components/AggregatesPanel';

function App() {
  const [page, setPage] = useState<'landing' | 'aggregates'>('landing');

  if (page === 'landing') {
    return <LandingPage onBegin={() => setPage('aggregates')} />;
  }

  return <AggregatesPanel />;
}

export default App;
