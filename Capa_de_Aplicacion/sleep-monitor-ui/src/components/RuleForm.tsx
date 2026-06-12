import { useState } from "react";
import { api } from "../api";

interface Lookup {
  id: number;
  name: string;
}

interface RuleFormProps {
  readings: Lookup[];
  conditions: Lookup[];
  actuators: Lookup[];
  onCreated: () => void;
}

export default function RuleForm({
  readings,
  conditions,
  actuators,
  onCreated
}: RuleFormProps) {
  // map backend names to friendly labels for display
  const readingMap: Record<string, string> = {
    temperature: "Temperatura",
    humidity: "Humedad",
    light: "Luz",
    noise: "Ruido",
    motion: "Movimiento"
  };
  const conditionMap: Record<string, string> = {
    over: "Mayor que",
    under: "Menor que",
    equal: "Igual a",
    different: "Diferente",
    over_or_equal: "Mayor o igual",
    under_or_equal: "Menor o igual"
  };

  const actuatorMap: Record<string, string> = {
    buzzer: "Alarma",
    led: "Luces",
    motor: "Ventanas"
  };

  // hide actuador 'led' because user cannot activate it currently
  const filteredActuators = actuators.filter((a: Lookup) => a.name !== "led");

  const [reading, setReading] = useState<number>(
    readings[0]?.id ?? 1
  );
  const [condition, setCondition] = useState<number>(
    conditions[0]?.id ?? 1
  );
  const [actuator, setActuator] = useState<number>(
    filteredActuators[0]?.id ?? actuators[0]?.id ?? 1
  );
  const [value, setValue] = useState("");

  async function submit() {
    await api.createRule({
      id: 0,
      id_reading_type: reading,
      id_condition_type: condition,
      id_actuator_type: actuator,
      condition_value: Number(value),
      created_at: ""
    });

    onCreated();
  }

  return (
    <div className="card">
      <h3>Crear Regla</h3>

      <div className="form">
        <div className="form-row">
          <label>Lectura</label>
          <select className="form-field" onChange={e => setReading(Number(e.target.value))} value={reading}>
            {readings.map((r: Lookup) => (
              <option key={r.id} value={r.id}>{readingMap[r.name] ?? r.name}</option>
            ))}
          </select>
        </div>

        <div className="form-row">
          <label>Condición</label>
          <select className="form-field" onChange={e => setCondition(Number(e.target.value))} value={condition}>
            {conditions.map((c: Lookup) => (
              <option key={c.id} value={c.id}>{conditionMap[c.name] ?? c.name}</option>
            ))}
          </select>
        </div>

        <div className="form-row">
          <label>Valor</label>
          <input className="form-field" type="number" value={value} onChange={e => setValue(e.target.value)} />
        </div>

        <div className="form-row">
          <label>Actuador</label>
          <select className="form-field" onChange={e => setActuator(Number(e.target.value))} value={actuator}>
            {filteredActuators.map((a: Lookup) => (
              <option key={a.id} value={a.id}>{actuatorMap[a.name] ?? a.name}</option>
            ))}
          </select>
          <div className="muted">Nota: las luces no están disponibles para activación</div>
        </div>

        <div className="form-row">
          <button className="btn" onClick={submit}>Crear</button>
        </div>
      </div>
    </div>
  );
}