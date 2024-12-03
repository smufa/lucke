import { Container, Stack, Title } from "@mantine/core";
import { BarCard } from "./BarCard";
import { useUpdateData } from "./useUpdateData";
function App() {
  const { data } = useUpdateData(1000);

  return (
    <Container>
      <Stack justify="center" py="xl">
        <Stack gap="xs">
          <Title
            style={{
              textAlign: "center",
            }}
          >
            LED Stat
          </Title>
          <Title
            style={{
              textAlign: "center",
            }}
            order={3}
          >
            Gerba 59
          </Title>
        </Stack>

        <Stack gap="2rem">
          {data?.map((bar) => (
            <BarCard
              lastHeartbeat={bar.lastHeartbeat}
              key={bar.universe + bar.local_ip}
              universe={bar.universe}
              heap_size={bar.heap_size}
              heap_free={bar.heap_free}
              local_ip={bar.local_ip}
              ssid={bar.ssid}
              rssi={bar.rssi}
              DMXFramerate={bar.DMXFramerate}
              first_5_leds={bar.first_5_leds}
            />
          ))}
        </Stack>
      </Stack>
    </Container>
  );
}

export default App;
