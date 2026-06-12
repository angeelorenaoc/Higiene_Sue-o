import { useEffect, useState } from "react";

import { api } from "./api";

import Dashboard from "./pages/Dashboard";
import Rules from "./pages/Rules";
import Activity from "./pages/Activity";
import Alarms from "./pages/Alarms";

export default function App() {
  const [tab, setTab] =
    useState("dashboard");

  const [readings, setReadings] =
    useState([]);

  const [rules, setRules] =
    useState([]);

  const [logs, setLogs] =
    useState([]);

  const [alarms, setAlarms] =
    useState([]);

  const [readingTypes,
    setReadingTypes] = useState([]);

  const [conditionTypes,
    setConditionTypes] = useState([]);

  const [actuatorTypes,
    setActuatorTypes] = useState([]);

  async function load() {
    const [
      readings,
      rules,
      logs,
      alarms,
      rt,
      ct,
      at
    ] = await Promise.all([
      api.readings(100),
      api.rules(),
      api.logs(),
        api.alarms(),
      api.readingTypes(),
      api.conditionTypes(),
      api.actuatorTypes()
    ]);

    setReadings(readings);
    setRules(rules);
    setLogs(logs);
      setAlarms(alarms);

    setReadingTypes(rt);
    setConditionTypes(ct);
    setActuatorTypes(at);
  }

  useEffect(() => {
    load();
  }, []);

  return (
    <div className="site">
      <div className="tabs">

        <button
          className={tab === "dashboard" ? "active" : ""}
          onClick={() => setTab("dashboard")}
        >
          Panel
        </button>

        <button
          className={tab === "rules" ? "active" : ""}
          onClick={() => setTab("rules")}
        >
          Reglas
        </button>

        <button
          className={tab === "alarms" ? "active" : ""}
          onClick={() => setTab("alarms")}
        >
          Alarmas
        </button>

        <button
          className={tab === "activity" ? "active" : ""}
          onClick={() => setTab("activity")}
        >
          Actividad
        </button>

        <div className="spacer" />

        <button onClick={load} className="btn">
          Actualizar
        </button>

      </div>

      {tab === "dashboard" && (
        <Dashboard readings={readings} />
      )}

      {tab === "rules" && (
        <Rules
          rules={rules}
          readings={readingTypes}
          conditions={conditionTypes}
          actuators={actuatorTypes}
          refresh={load}
        />
      )}

      {tab === "alarms" && (
        <Alarms
          alarms={alarms}
          conditions={conditionTypes}
          actuators={actuatorTypes}
          refresh={load}
        />
      )}

      {tab === "activity" && (
        <Activity
          logs={logs}
          actuators={actuatorTypes}
          readings={readings}
          rules={rules}
          readingTypes={readingTypes}
          conditionTypes={conditionTypes}
        />
      )}
    </div>
  );
}