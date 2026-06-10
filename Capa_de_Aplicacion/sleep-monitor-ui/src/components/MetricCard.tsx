interface Props {
  title: string;
  value: string;
}

export default function MetricCard({
  title,
  value
}: Props) {
  return (
    <div className="card">
      <h3>{title}</h3>
      <h1>{value}</h1>
    </div>
  );
}