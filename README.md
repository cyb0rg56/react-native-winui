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

## Shared behavior

Every control accepts React Native `View` layout props, plus:

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `theme` | `'light' \| 'dark' \| 'system'` | follows the React Native color scheme | `'system'` leaves the WinUI theme alone. While Windows high contrast is on, the control uses the system theme. That is checked again when props are applied. |
| `disabled` | `boolean` | `false` | |
| `accessibilityLabel` | `string` | | Forwarded to the WinUI automation name. |
| `testID` | `string` | | Forwarded to the WinUI automation id. |

Interactive controls are controlled. A user edit fires `onChange` and the matching `on*` event, then the native control returns to the current prop. Update the prop from the callback for the new value to remain.

Give a control a `style` width and height when the first frame must be stable. After the control measures itself, layout uses that desired size.

```tsx
import { ComboBox } from 'react-native-winui';

<ComboBox
  accessibilityLabel="Language"
  style={{ width: 220, height: 32 }}
  items={[
    { label: 'C++', value: 'cpp' },
    { label: 'TypeScript', value: 'ts' },
  ]}
  selectedIndex={index}
  onChange={(nextIndex) => setIndex(nextIndex)}
/>
```

## Components

### ComboBox

WinUI `ComboBox`. `items` is required.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `items` | `{ label: string; value?: string }[]` | | `value` falls back to `label` in the event payload. |
| `selectedIndex` | `number` | `-1` | No selection. |
| `placeholder` | `string` | | |
| `onChange` | `(index: number, item?: ComboBoxItem) => void` | | |
| `onSelect` | event | | `nativeEvent` is `{ value, itemIndex, text }`. |

### NumberBox

WinUI `NumberBox`. Omit `value`, or pass `null`, to show an empty field.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `value` | `number \| null` | empty | |
| `minimum` | `number` | WinUI minimum | Removing the prop restores the WinUI default. |
| `maximum` | `number` | WinUI maximum | Removing the prop restores the WinUI default. |
| `step` | `number` | `1` | Sets `SmallChange`. |
| `spinButtons` | `'hidden' \| 'compact' \| 'inline'` | `'compact'` | |
| `placeholder` | `string` | | |
| `onChange` | `(value: number \| null) => void` | | `null` when the field is cleared. |
| `onValueChange` | event | | `nativeEvent` is `{ value, isEmpty }`. `value` is `0` when `isEmpty` is true. |

### CheckBox

WinUI `CheckBox`.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `label` | `string` | | |
| `checked` | `boolean` | `false` | |
| `indeterminate` | `boolean` | `false` | When true, the box is in the indeterminate state. |
| `onChange` | `(checked: boolean, indeterminate: boolean) => void` | | |
| `onCheckedChange` | event | | `nativeEvent` is `{ checked, indeterminate }`. |

### RadioButton

WinUI `RadioButton`. Each control is its own XAML island, so `group` does not uncheck sibling radios. Use `RadioGroup` for a selection.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `label` | `string` | | |
| `checked` | `boolean` | `false` | |
| `group` | `string` | | WinUI `GroupName`. Scoped to this island. |
| `onChange` | `(checked: boolean) => void` | | |
| `onCheckedChange` | event | | `nativeEvent` is `{ checked }`. |

### RadioGroup

A JavaScript group that owns the selected value and checks one `RadioButton` per option.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `options` | `{ label: string; value: string; disabled?: boolean }[]` | | Required. |
| `value` | `string` | | The selected option value. |
| `onChange` | `(value: string) => void` | | Fires when an option is checked. |
| `disabled` | `boolean` | `false` | Disables every option. An option can also set its own `disabled`. |
| `theme` | `'light' \| 'dark' \| 'system'` | follows the color scheme | |
| `style` | `ViewStyle` | | Applied to the wrapping `View`. |

```tsx
<RadioGroup
  value={pane}
  onChange={setPane}
  options={[
    { label: 'Left', value: 'left' },
    { label: 'Top', value: 'top' },
  ]}
/>
```

### RatingControl

WinUI `RatingControl`.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `value` | `number` | `0` | |
| `maxRating` | `number` | `5` | |
| `onChange` | `(value: number) => void` | | |
| `onValueChange` | event | | `nativeEvent` is `{ value }`. |

### InfoBar

WinUI `InfoBar`. Closing is cancelled until `isOpen` changes, so the bar stays under React's control.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `title` | `string` | | |
| `message` | `string` | | |
| `severity` | `'informational' \| 'success' \| 'warning' \| 'error'` | `'informational'` | |
| `isOpen` | `boolean` | `true` | A closed bar measures to zero height. |
| `isClosable` | `boolean` | `true` | |
| `onClose` | event | | Close was requested. `nativeEvent.isOpen` is `false`. |

### InfoBadge

WinUI `InfoBadge`.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `value` | `number` | `-1` | `-1` draws a dot with no count. |

### ProgressBar

WinUI `ProgressBar`.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `value` | `number` | `0` | Ignored while `isIndeterminate` is true. |
| `minimum` | `number` | `0` | |
| `maximum` | `number` | `100` | |
| `isIndeterminate` | `boolean` | `false` | |

### Expander

WinUI `Expander`. `header` and `content` are strings. React children are not mounted inside the island.

| Prop | Type | Default | Notes |
| --- | --- | --- | --- |
| `header` | `string` | | |
| `content` | `string` | | |
| `isExpanded` | `boolean` | `false` | |
| `onChange` | `(isExpanded: boolean) => void` | | |
| `onExpandChange` | event | | `nativeEvent` is `{ isExpanded }`. |

## Example

```sh
yarn install
yarn example windows
```
