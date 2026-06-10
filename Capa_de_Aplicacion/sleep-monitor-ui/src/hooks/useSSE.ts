import { useEffect } from "react";

export function useSSE(
 callback: (data: unknown) => void
) {
  useEffect(() => {
    const es = new EventSource(
      "http://localhost:8000/api/v0/events"
    );

    es.onmessage = e => {
      callback(JSON.parse(e.data));
    };

    return () => es.close();
  });
}