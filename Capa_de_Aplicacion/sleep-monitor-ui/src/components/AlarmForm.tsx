import { useState } from "react";
import { api } from "../api";

interface Lookup {
  id: number;
  name: string;
}

interface AlarmFormProps {
  conditions: Lookup[];
  actuators: Lookup[];
  onCreated: () => void;
}

export default function AlarmForm({
  conditions,
  actuators,
  onCreated
}: AlarmFormProps) {
  const [condition, setCondition] = useState<number>(conditions[0]?.id ?? 1);
  const [actuator, setActuator] = useState<number>(
    actuators.find(a => a.name !== "led")?.id ?? actuators[0]?.id ?? 1
  );
  const [time, setTime] = useState("00:00");
  
  function normalizeTime(t: string) {
    const s = (t || "").trim();
    if (!s) return "00:00:00";

    // am/pm handling
    const ampm = /\b(am|pm)\b/i.exec(s);
    if (ampm) {
      const isPM = /pm/i.test(ampm[0]);
      const cleaned = s.replace(/\s*(am|pm)\s*/i, "").trim();
      const parts = cleaned.split(/[:\s]+/).filter(Boolean);
      let h = Number(parts[0] ?? 0);
      const m = Number(parts[1] ?? 0);
      const sec = Number(parts[2] ?? 0);
      if (isPM && h < 12) h += 12;
      if (!isPM && h === 12) h = 0;
      return [h, m, sec].map(n => String(n).padStart(2, "0")).join(":");
    }

    const parts = s.split(":").map(p => p.trim());
    if (parts.length === 2) {
      const h = Number(parts[0] ?? 0);
      const m = Number(parts[1] ?? 0);
      return `${String(h).padStart(2, "0")}:${String(m).padStart(2, "0")}:00`;
    }

    if (parts.length === 3) {
      const h = Number(parts[0] ?? 0);
      const m = Number(parts[1] ?? 0);
      const sec = Number(parts[2] ?? 0);
      return `${String(h).padStart(2, "0")}:${String(m).padStart(2, "0")}:${String(sec).padStart(2, "0")}`;
    }

    // fallback: try Date parse
    const d = new Date(`1970-01-01T${s}`);
    if (!isNaN(d.getTime())) {
      const hh = d.getHours();
      const mm = d.getMinutes();
      const ss = d.getSeconds();
      return `${String(hh).padStart(2, "0")}:${String(mm).padStart(2, "0")}:${String(ss).padStart(2, "0")}`;
    }

    return "00:00:00";
  }

  async function submit() {
    const normalized = normalizeTime(time);
    await api.createAlarm({
      id: 0,
      id_condition_type: condition,
      id_actuator_type: actuator,
      condition_time: normalized,
      created_at: ""
    });

    onCreated();
  }

  return (
    <div className="card">
      <h3>Crear Alarma</h3>

      <div className="form">
        <div className="form-row">
          <label>Condición</label>
          <select
            className="form-field"
            onChange={e => setCondition(Number(e.target.value))}
            value={condition}
          >
              {(() => {
                const conditionMap: Record<string, string> = {
                  over: "Mayor que",
                  under: "Menor que",
                  equal: "Igual a",
                  different: "Diferente",
                  over_or_equal: "Mayor o igual",
                  under_or_equal: "Menor o igual"
                };

                return conditions.map(c => (
                  <option key={c.id} value={c.id}>{conditionMap[c.name] ?? c.name}</option>
                ));
              })()}
          </select>
        </div>

        <div className="form-row">
          <label>Hora</label>
          <input
            className="form-field"
            type="time"
            value={time}
            onChange={e => setTime(e.target.value)}
          />
          <div className="muted">Se enviará al servidor como hh:mm:ss</div>
        </div>

        <div className="form-row">
          <label>Actuador</label>
          <select
            className="form-field"
            onChange={e => setActuator(Number(e.target.value))}
            value={actuator}
          >
            {actuators
              .filter(a => a.name !== "led")
              .map(a => (
                <option key={a.id} value={a.id}>{
                  a.name === "buzzer" ? "Alarma" : a.name === "motor" ? "Ventanas" : a.name
                }</option>
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
