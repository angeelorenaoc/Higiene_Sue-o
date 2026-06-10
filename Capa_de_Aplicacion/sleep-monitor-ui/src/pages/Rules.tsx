import RuleForm from "../components/RuleForm";
import { api } from "../api";

export default function Rules(props: any) {
  const {
    rules,
    readings,
    conditions,
    actuators,
    refresh
  } = props;

  const rt = Object.fromEntries(
    readings.map((x: any) => [
      x.id,
      x.name
    ])
  );

  const ct = Object.fromEntries(
    conditions.map((x: any) => [
      x.id,
      x.name
    ])
  );

  const at = Object.fromEntries(
    actuators.map((x: any) => [
      x.id,
      x.name
    ])
  );

  return (
    <div className="page">

      <RuleForm
        readings={readings}
        conditions={conditions}
        actuators={actuators}
        onCreated={refresh}
      />

      <table>
        <thead>
          <tr>
            <th>Reading</th>
            <th>Condition</th>
            <th>Value</th>
            <th>Actuator</th>
            <th />
          </tr>
        </thead>

        <tbody>
          {rules.map((r: any) => (
            <tr key={r.id}>
              <td>
                {
                  rt[
                    r.id_reading_type
                  ]
                }
              </td>

              <td>
                {
                  ct[
                    r.id_condition_type
                  ]
                }
              </td>

              <td>
                {r.condition_value}
              </td>

              <td>
                {
                  at[
                    r.id_actuator_type
                  ]
                }
              </td>

              <td>
                <button
                  onClick={async () => {
                    await api.deleteRule(
                      r.id
                    );

                    refresh();
                  }}
                >
                  Delete
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>

    </div>
  );
}