import { Card, Stack, Title } from "@mantine/core";
import { stats } from "./useUpdateData";

export const GraphCard = ({
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
  return (
    <Card withBorder>
      <Stack>
        <Title>Grafi</Title>
      </Stack>
    </Card>
  );
};
