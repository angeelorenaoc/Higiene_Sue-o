interface Props {
  title: string;
  current: string;
  average?: string;
}

export default function MetricCard({
  title,
  current,
  average
}: Props) {
  return (
    <div className="card small metric-card">
      <h3>{title}</h3>

      <div className="metric-main">
        <div className="metric-current">
          <span className="label">Current</span>
          <span className="value">{current}</span>
        </div>

        {average && (
          <div className="metric-avg">
            <span className="label">Average</span>
            <span className="value">{average}</span>
          </div>
        )}
      </div>
    </div>
  );
}