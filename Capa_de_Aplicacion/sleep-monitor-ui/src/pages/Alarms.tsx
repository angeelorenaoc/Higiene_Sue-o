import AlarmForm from "../components/AlarmForm";
import { api } from "../api";

export default function Alarms(props: any) {
  const {
    alarms,
    conditions,
    actuators,
    refresh
  } = props;

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

  const ct = Object.fromEntries(
    conditions.map((x: any) => [x.id, conditionMap[x.name] ?? x.name])
  );

  const at = Object.fromEntries(
    actuators.map((x: any) => [x.id, actuatorMap[x.name] ?? x.name])
  );

  return (
    <div className="page">
      <div className="cols">
        <div>
          <AlarmForm
            conditions={conditions}
            actuators={actuators}
            onCreated={refresh}
          />
        </div>

        <div className="card">
          <div className="card-header">
            <div>
              <strong>Alarmas</strong>
              <div className="muted">Acciones programadas por tiempo</div>
            </div>
            <div />
          </div>

          <table>
            <thead>
              <tr>
                <th>Condición</th>
                <th>Hora</th>
                <th>Actuador</th>
                <th />
              </tr>
            </thead>

            <tbody>
              {alarms.map((a: any) => (
                <tr key={a.id}>
                  <td>{ct[a.id_condition_type]}</td>
                  <td>{a.condition_time}</td>
                  <td>{at[a.id_actuator_type]}</td>
                  <td>
                    <button
                      className="btn danger"
                      onClick={async () => {
                        await api.deleteAlarm(a.id);
                        refresh();
                      }}
                    >
                      Eliminar
                    </button>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      </div>

    </div>
  );
}
