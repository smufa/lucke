export function getColor(
  limits: number[],
  colors: string[],
  value: number
): string {
  if (value <= limits[0]) {
    return colors[0];
  }

  for (let i = 1; i < limits.length - 1; i++) {
    if (value >= limits[i] && value <= limits[i + 1]) {
      return colors[i + 1];
    }
  }

  // If the value exceeds all limits, return the last color
  return colors[colors.length - 1];
}
