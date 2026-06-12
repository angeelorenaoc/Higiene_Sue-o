import MetricCard from "../components/MetricCard";
import ReadingChart from "../components/ReadingChart";
import type { Reading } from "../types";

export default function Dashboard({
  readings
}: any) {
  const latest = (id: number) =>
    readings.find(
      (r: Reading) =>
        r.id_reading_type === id
    );

  function avgFor(id: number) {
    const arr = readings
      .filter((r: Reading) => r.id_reading_type === id)
      .map((r: Reading) => Number(r.value));

    if (!arr.length) return 0;
    return arr.reduce((s: number, v: number) => s + v, 0) / arr.length;
  }

  const thresholds = {
    temperature: 20,
    humidity: 50,
    noise: 30,
    light: 10
  };

  return (
    <div className="page">

      <div className="grid metrics">

        <MetricCard
          title="Temperatura"
          current={`${latest(1)?.value ?? '-'} °C`}
          average={`${avgFor(1).toFixed(1)} °C`}
        />

        <MetricCard
          title="Humedad"
          current={`${latest(2)?.value ?? '-'} %`}
          average={`${avgFor(2).toFixed(1)} %`}
        />

        <MetricCard
          title="Luz"
          current={`${String(latest(3)?.value ?? 0)} %`}
          average={`${avgFor(3).toFixed(1)} %`}
        />

        <MetricCard
          title="Ruido"
          current={`${String(latest(4)?.value ?? 0)} dB`}
          average={`${avgFor(4).toFixed(1)} dB`}
        />

      </div>

      <div className="grid charts">
        <div>
          <ReadingChart
            title="Temperatura"
            data={readings.filter((r: Reading) => r.id_reading_type === 1)}
          />
          {avgFor(1) > thresholds.temperature && (
            <div className="recommendation">
              La temperatura media es {avgFor(1).toFixed(1)} °C — reduce a alrededor de {thresholds.temperature} °C para mejorar el sueño.
            </div>
          )}
        </div>

        <div>
          <ReadingChart
            title="Humedad"
            data={readings.filter((r: Reading) => r.id_reading_type === 2)}
          />
          {avgFor(2) > thresholds.humidity && (
            <div className="recommendation">
              La humedad media es {avgFor(2).toFixed(1)} % — intenta un valor cercano a {thresholds.humidity}% para mejorar el sueño.
            </div>
          )}
        </div>

        <div>
          <ReadingChart
            title="Luz"
            data={readings.filter((r: Reading) => r.id_reading_type === 3)}
          />
          {avgFor(3) > thresholds.light && (
            <div className="recommendation">
              La luz media es {avgFor(3).toFixed(1)} % — reduce la luz ambiental (objetivo &lt; {thresholds.light}%) para mejorar el sueño.
            </div>
          )}
        </div>

        <div>
          <ReadingChart
            title="Ruido"
            data={readings.filter((r: Reading) => r.id_reading_type === 4)}
          />
          {avgFor(4) > thresholds.noise && (
            <div className="recommendation">
              El ruido medio es {avgFor(4).toFixed(1)} dB — intenta mantener el ruido por debajo de {thresholds.noise} dB para un sueño óptimo.
            </div>
          )}
        </div>
      </div>
    </div>
  );
}