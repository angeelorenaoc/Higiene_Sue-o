import { useEffect, useState } from "react";

import { api } from "./api";

import Dashboard from "./pages/Dashboard";
import Rules from "./pages/Rules";
import Activity from "./pages/Activity";

export default function App() {
  const [tab, setTab] =
    useState("dashboard");

  const [readings, setReadings] =
    useState([]);

  const [rules, setRules] =
    useState([]);

  const [logs, setLogs] =
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
      rt,
      ct,
      at
    ] = await Promise.all([
      api.readings(100),
      api.rules(),
      api.logs(),
      api.readingTypes(),
      api.conditionTypes(),
      api.actuatorTypes()
    ]);

    setReadings(readings);
    setRules(rules);
    setLogs(logs);

    setReadingTypes(rt);
    setConditionTypes(ct);
    setActuatorTypes(at);
  }

  useEffect(() => {
    load();

    const timer =
      setInterval(load, 5000);

    return () =>
      clearInterval(timer);
  }, []);

  return (
    <>
      <div className="tabs">

        <button
          onClick={() =>
            setTab("dashboard")
          }
        >
          Dashboard
        </button>

        <button
          onClick={() =>
            setTab("rules")
          }
        >
          Rules
        </button>

        <button
          onClick={() =>
            setTab("activity")
          }
        >
          Activity
        </button>

      </div>

      {tab === "dashboard" && (
        <Dashboard
          readings={readings}
        />
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

      {tab === "activity" && (
        <Activity
          logs={logs}
          actuators={actuatorTypes}
        />
      )}
    </>
  );
}