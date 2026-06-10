import { useState } from "react";
import { api } from "../api";

export default function RuleForm({
  readings,
  conditions,
  actuators,
  onCreated
}: any) {
  const [reading, setReading] = useState(1);
  const [condition, setCondition] = useState(1);
  const [actuator, setActuator] = useState(1);
  const [value, setValue] = useState("");

  async function submit() {
    await api.createRule({
      id_reading_type: reading,
      id_condition_type: condition,
      id_actuator_type: actuator,
      condition_value: Number(value)
    });

    onCreated();
  }

  return (
    <div className="card">
      <h3>Create Rule</h3>

      <select
        onChange={e =>
          setReading(Number(e.target.value))
        }
      >
        {readings.map((r: any) => (
          <option
            key={r.id}
            value={r.id}
          >
            {r.name}
          </option>
        ))}
      </select>

      <select
        onChange={e =>
          setCondition(Number(e.target.value))
        }
      >
        {conditions.map((c: any) => (
          <option
            key={c.id}
            value={c.id}
          >
            {c.name}
          </option>
        ))}
      </select>

      <input
        type="number"
        value={value}
        onChange={e =>
          setValue(e.target.value)
        }
      />

      <select
        onChange={e =>
          setActuator(Number(e.target.value))
        }
      >
        {actuators.map((a: unknown) => (
          <option
            key={a.id}
            value={a.id}
          >
            {a.name}
          </option>
        ))}
      </select>

      <button onClick={submit}>
        Create
      </button>
    </div>
  );
}