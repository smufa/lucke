import { useEffect, useState } from "react";

export interface DataIn {
  [key: string]: stats;
}

export interface stats {
  universe: number;
  heap_size: number;
  heap_free: number;
  local_ip: string;
  ssid: string;
  rssi: number[];
  DMXFramerate: number[];
  first_5_leds: number[];
  lastHeartbeat: string;
}

export const useUpdateData = (intervalMs: number) => {
  const [data, setData] = useState<stats[]>();

  const updateData = () => {
    fetch(`http://localhost:8080`)
      .then(async (data) => {
        const dataJson = (await data.json()) as unknown as DataIn;
        setData(Object.values(dataJson));
      })
      .catch(() => {});
  };

  useEffect(() => {
    const interval = setInterval(() => {
      updateData();
    }, intervalMs);

    // clean after yourself
    return () => {
      clearInterval(interval);
    };
  }, [intervalMs]);

  return {
    data,
  };
};
