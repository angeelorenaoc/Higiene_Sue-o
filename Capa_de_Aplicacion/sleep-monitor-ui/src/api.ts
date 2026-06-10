const API = "http://localhost:8000/api/v0";

export const api = {
  readings: (limit = 100) =>
    fetch(`${API}/readings?limit=${limit}`).then(r => r.json()),

  rules: () =>
    fetch(`${API}/rules`).then(r => r.json()),

  logs: () =>
    fetch(`${API}/actuator-logs`).then(r => r.json()),

  readingTypes: () =>
    fetch(`${API}/reading-types`).then(r => r.json()),

  conditionTypes: () =>
    fetch(`${API}/condition-types`).then(r => r.json()),

  actuatorTypes: () =>
    fetch(`${API}/actuator-types`).then(r => r.json()),

  createRule: (body: unknown) =>
    fetch(`${API}/rules`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify(body)
    }),

  deleteRule: (id: number) =>
    fetch(`${API}/rules/${id}`, {
      method: "DELETE"
    })
};