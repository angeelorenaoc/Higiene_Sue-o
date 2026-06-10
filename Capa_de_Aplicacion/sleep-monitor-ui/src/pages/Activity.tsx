export default function Activity({
  logs,
  actuators
}: any) {
  const names = Object.fromEntries(
    actuators.map((a: any) => [
      a.id,
      a.name
    ])
  );

  return (
    <div className="page">
      <table>
        <thead>
          <tr>
            <th>Date</th>
            <th>Actuator</th>
            <th>Command</th>
          </tr>
        </thead>

        <tbody>
          {logs.map((l: any) => (
            <tr key={l.id}>
              <td>{l.created_at}</td>
              <td>
                {
                  names[
                    l.id_actuator_type
                  ]
                }
              </td>
              <td>{l.command}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}