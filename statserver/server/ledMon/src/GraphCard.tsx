import { Card, Stack, Title } from "@mantine/core";
import { LineChart } from "@mantine/charts";
import { stats } from "./useUpdateData";

type statKey = "rssi" | "DMXFramerate" | "nm_dropped";
const formatData = (data: stats[], key: statKey) => {
  const dataOut = [];
  const len = data[0][key].length;

  for (let index = 0; index < len; index++) {
    const out = {};
    Object.values(data).forEach((val) => {
      const uni = val.universe;
      // @ts-expect-error delamo unsafe mučke
      out[uni] = val[key][index];
    });
    dataOut.push(out);
  }
  return dataOut;
};

interface GraphCardProps {
  data: stats[];
}
export const GraphCard = ({ data }: GraphCardProps) => {
  return (
    <Card withBorder>
      <Stack>
        <Title>Grafi</Title>
        <LineChart
          title="RSSI"
          h={200}
          data={formatData(data, "rssi")}
          dataKey="rssi"
          series={data.map((val) => ({ name: val.universe.toString() }))}
          curveType="linear"
        />

        <LineChart
          title="Framerate"
          h={200}
          data={formatData(data, "DMXFramerate")}
          dataKey="DMXFramerate"
          series={data.map((val) => ({ name: val.universe.toString() }))}
          curveType="linear"
        />

        <LineChart
          title="Dropped packets"
          h={200}
          data={formatData(data, "nm_dropped")}
          dataKey="nm_dropped"
          series={data.map((val) => ({ name: val.universe.toString() }))}
          curveType="linear"
        />
      </Stack>
    </Card>
  );
};
