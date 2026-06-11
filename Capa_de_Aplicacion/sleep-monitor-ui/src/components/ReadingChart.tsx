import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  ResponsiveContainer
} from "recharts";

interface ChartPoint {
  created_at: string;
  value: number;
}

interface ReadingChartProps {
  title: string;
  data: ChartPoint[];
}

export default function ReadingChart({
  title,
  data
}: ReadingChartProps) {
  return (
    <div className="card">
      <h3>{title}</h3>

      <ResponsiveContainer
        width="100%"
        height={300}
      >
        <LineChart data={data}>
          <XAxis dataKey="created_at" />
          <YAxis />
          <Tooltip />
          <Line
            dataKey="value"
            stroke="#8b5cf6"
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}