# react-native-winui

WinUI 3 controls for React Native Windows. Windows-only. New Architecture only.

Each component is a Fabric view that hosts one `Microsoft.UI.Xaml.Controls` control in a XAML island. Importing any control initializes WinUI for the process.

## Requirements

- `react-native-windows` >= 0.82.0. Developed against 0.84.
- Visual Studio 2026, which React Native Windows 0.84 requires.
- A Fabric app (`cpp-app`). Paper is not supported.

## Install

```sh
yarn add react-native-winui
```

Autolinking picks up `windows/Winui/Winui.vcxproj`. Rebuild the Windows app after installing.

## Controls

`ComboBox`, `NumberBox`, `CheckBox`, `RadioButton`, `RatingControl`, `InfoBar`, `InfoBadge`, `ProgressBar`, `Expander`.

`Expander` takes `header` and `content` strings. It does not mount React children inside the island.

Omit `theme` to follow the React Native color scheme. Pass `theme="system"` to leave the WinUI theme alone. When Windows high contrast is on, the control stays on the system theme.

```tsx
import { ComboBox } from 'react-native-winui';

<ComboBox
  style={{ width: 220, height: 36 }}
  items={[
    { label: 'C++', value: 'cpp' },
    { label: 'TypeScript', value: 'ts' },
  ]}
  selectedIndex={index}
  onSelect={(event) => setIndex(event.nativeEvent.itemIndex)}
/>
```

Give a control a `style` width and height when the first frame must be stable. After the control measures itself, layout uses that desired size.

## Example

```sh
yarn install
yarn example windows
```
