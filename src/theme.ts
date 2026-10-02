import { useColorScheme, type ColorSchemeName } from 'react-native';

export type WinuiTheme = 'light' | 'dark' | 'system';

export function resolveWinuiTheme(
  theme: WinuiTheme | null | undefined,
  colorScheme: ColorSchemeName | null
): WinuiTheme {
  if (theme === 'light' || theme === 'dark' || theme === 'system') {
    return theme;
  }
  return colorScheme === 'dark' ? 'dark' : 'light';
}

export function useWinuiTheme(
  theme: WinuiTheme | null | undefined
): WinuiTheme {
  return resolveWinuiTheme(theme, useColorScheme());
}
