import { Group, ColorSwatch } from "@mantine/core";
import { useMemo } from "react";

export interface LedDisplayProps {
  leds: number[];
}

interface LED {
  R: number;
  G: number;
  B: number;
}

export const LedDisplay = ({ leds }: LedDisplayProps) => {
  const colors = useMemo(() => {
    const l: LED[] = [];
    for (let i = 0; i < leds.length; i += 3) {
      l.push({
        R: leds[i],
        G: leds[i + 1],
        B: leds[i + 2],
      });
    }

    return l;
  }, [leds]);

  return (
    <Group justify="space-evenly" py="xs">
      {colors.map((color, i) => (
        <ColorSwatch
          key={i}
          color={`rgb(${color.R},${color.G},${color.B})`}
          size="1.3rem"
        />
      ))}
    </Group>
  );
};
