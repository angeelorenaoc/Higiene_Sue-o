import type { ActuatorLog, Lookup, Reading, Rule } from "../types";

export default function Activity({
  logs,
  actuators,
  readings,
  rules,
  readingTypes,
  conditionTypes
}: any) {
  const actuatorMap: Record<string, string> = {
    buzzer: "Alarma",
    led: "Luces",
    motor: "Ventanas"
  };

  const readingNameMap: Record<string, string> = {
    temperature: "Temperatura",
    humidity: "Humedad",
    light: "Luz",
    noise: "Ruido",
    motion: "Movimiento"
  };

  const conditionNameMap: Record<string, string> = {
    over: "Mayor que",
    under: "Menor que",
    equal: "Igual a",
    different: "Diferente",
    over_or_equal: "Mayor o igual",
    under_or_equal: "Menor o igual"
  };

  const readingTypeMap: Record<number, string> = Object.fromEntries(
    readingTypes.map((r: Lookup) => [r.id, readingNameMap[r.name] ?? r.name])
  );

  const conditionMap: Record<number, string> = Object.fromEntries(
    conditionTypes.map((c: Lookup) => [c.id, conditionNameMap[c.name] ?? c.name])
  );

  const ruleMap: Record<number, Rule> = Object.fromEntries(
    rules.map((r: Rule) => [r.id, r])
  );

  const names = Object.fromEntries(
    actuators.map((a: Lookup) => [a.id, actuatorMap[a.name] ?? a.name])
  );

  return (
    <div className="page">
      <div className="card">
        <div className="card-header">
          <div>
            <strong>Actividad</strong>
            <div className="muted">Comandos recientes de actuadores</div>
          </div>
        </div>

        <table className="activity-table">
          <thead>
            <tr>
              <th>Fecha</th>
              <th>Actuador</th>
              <th>Regla</th>
              <th>Lectura</th>
            </tr>
          </thead>

        <tbody>
          {logs.map((l: ActuatorLog) => {
            // find rule dynamically in case rules loaded later or ids mismatch
            const rule = (rules || []).find((r: Rule) => r.id === l.id_rule);
            const reading = (readings || []).find((r: Reading) => r.id === l.id_reading);

            let ruleText: string;
            if (rule) {
              // Prefer maps, but if they don't contain the id try to find entries
              // directly in the provided lookup arrays and map their backend
              // names to friendly labels using readingNameMap / conditionNameMap.
              let readingTypeName = readingTypeMap[rule.id_reading_type] ?? String(rule.id_reading_type);
              if (readingTypeName === String(rule.id_reading_type)) {
                const rt = (readingTypes || []).find((r: Lookup) => r.id === rule.id_reading_type);
                if (rt) readingTypeName = readingNameMap[rt.name] ?? rt.name;
              }

              let conditionName = conditionMap[rule.id_condition_type] ?? String(rule.id_condition_type);
              if (conditionName === String(rule.id_condition_type)) {
                const ct = (conditionTypes || []).find((c: Lookup) => c.id === rule.id_condition_type);
                if (ct) conditionName = conditionNameMap[ct.name] ?? ct.name;
              }

              ruleText = `${readingTypeName} ${conditionName} ${rule.condition_value} (No. ${rule.id})`;
            } else {
              // If the referenced rule was deleted, show a clear fallback
              ruleText = l.id_rule ? `Regla eliminada (No. ${l.id_rule})` : "-";
            }

            const readingText = reading ? `${reading.value}` : String(l.id_reading ?? "-");

            return (
              <tr key={l.id}>
                <td className="col-date">{l.created_at}</td>
                <td className="col-actuator">{names[l.id_actuator_type]}</td>
                <td className="col-rule">{ruleText}</td>
                <td className="col-reading">{readingText}</td>
              </tr>
            );
          })}
        </tbody>
      </table>
      </div>
    </div>
  );
}