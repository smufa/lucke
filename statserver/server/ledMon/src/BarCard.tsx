import { Card, Group, Stack, Text } from "@mantine/core";
import { LineChart } from "@mantine/charts";
import { LedDisplay } from "./LedDisplay";
import { stats } from "./useUpdateData";
import { getColor } from "./colorUtils";
import { timeElapsed, timeElapsedPreety } from "./timeUtils";

export const BarCard = ({
  heap_free,
  heap_size,
  DMXFramerate,
  first_5_leds,
  local_ip,
  rssi,
  ssid,
  universe,
  lastHeartbeat,
}: stats) => {
  const meanRssi =
    rssi.reduce((partialSum, a) => partialSum + a, 0) / rssi.length;
  const rssiColor = getColor(
    [-80, -70, -67, -30],
    ["green.4", "green.4", "yellow.4", "orange.4", "red.4"],
    meanRssi
  );

  const lastHeartbeatColor = getColor(
    [5, 6, 7],
    ["green.4", "green.4", "yellow.4", "red.4"],
    timeElapsed(lastHeartbeat)
  );

  const meanDMXFramerate =
    DMXFramerate.reduce((partialSum, a) => partialSum + a, 0) /
    DMXFramerate.length;

  return (
    <Card withBorder>
      <Stack gap="xl">
        <Group justify="space-around">
          <Stack gap="0">
            <Text fw="bold" c="dimmed" size="xs">
              HEARTBEAT
            </Text>
            <Text fw="lighter" c={lastHeartbeatColor} size="sm">
              {timeElapsedPreety(lastHeartbeat)}
            </Text>
          </Stack>
          <Stack gap="0">
            <Text fw="bold" c="dimmed" size="xs">
              FIRST 5 LEDS
            </Text>
            <LedDisplay leds={first_5_leds} />
          </Stack>
          <Stack gap="0.1rem">
            <Text fw="bold" size="xl">
              Uni: {universe}
            </Text>
            <Text fw="lighter" c="dimmed" size="sm">
              {local_ip}
            </Text>
          </Stack>
        </Group>

        <Stack>
          <Group>
            <Text fw="bold" size="md" opacity={0.4}>
              RSSI:
            </Text>
            <Text size="md" c={rssiColor}>
              {meanRssi.toFixed(2)} db
            </Text>
          </Group>
          <LineChart
            h={100}
            title="RSSI"
            data={rssi.map((val) => {
              return {
                rssi: val,
              };
            })}
            dataKey="rssi"
            withYAxis={false}
            series={[{ name: "rssi", color: "indigo.6" }]}
            curveType="linear"
          />
        </Stack>

        <Stack>
          <Group>
            <Text fw="bold" size="md" opacity={0.4}>
              DMX framerate:
            </Text>
            <Text size="lg">{meanDMXFramerate.toFixed(2)}</Text>
          </Group>
          <LineChart
            h={100}
            title="DMX framerate"
            // data={data}
            data={DMXFramerate.map((val) => {
              return {
                framerate: val,
              };
            })}
            dataKey="framerate"
            withYAxis={false}
            series={[{ name: "framerate", color: "teal.6" }]}
            curveType="linear"
          />
        </Stack>

        <Group justify="space-around">
          <Stack gap="0">
            <Text fw="bold" c="dimmed" size="xs">
              HEAP FREE
            </Text>
            <Text c="dimmed" size="lg">
              {heap_free} b
            </Text>
          </Stack>

          <Stack gap="0">
            <Text fw="bold" c="dimmed" size="xs">
              HEAP SIZE
            </Text>
            <Text c="dimmed" size="lg">
              {heap_size} b
            </Text>
          </Stack>

          <Stack gap="0">
            <Text fw="bold" c="dimmed" size="xs">
              SSID
            </Text>
            <Text c="dimmed" size="lg">
              {ssid}
            </Text>
          </Stack>
        </Group>
      </Stack>
    </Card>
  );
};
