import MetricCard from "../components/MetricCard";
import ReadingChart from "../components/ReadingChart";

export default function Dashboard({
  readings
}: any) {
  const latest = (id: number) =>
    readings.find(
      (r: any) =>
        r.id_reading_type === id
    );

  return (
    <div className="page">

      <div className="grid metrics">

        <MetricCard
          title="Temperature"
          value={
            latest(1)?.value + " °C"
          }
        />

        <MetricCard
          title="Humidity"
          value={
            latest(2)?.value + " %"
          }
        />

        <MetricCard
          title="Light"
          value={
            String(latest(3)?.value ?? 0)
          }
        />

        <MetricCard
          title="Noise"
          value={
            String(latest(4)?.value ?? 0)
          }
        />

      </div>

      <ReadingChart
        title="Temperature"
        data={readings.filter(
          (r: any) =>
            r.id_reading_type === 1
        )}
      />
    </div>
  );
}