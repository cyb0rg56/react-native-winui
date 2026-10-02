import type { ComponentRef, Ref } from 'react';
import {
  View,
  type StyleProp,
  type ViewProps,
  type ViewStyle,
} from 'react-native';
import NativeCheckBox, {
  type CheckBoxChangeEvent,
} from './CheckBoxNativeComponent';
import NativeComboBox, {
  type ComboBoxChangeEvent,
  type ComboBoxItem,
} from './ComboBoxNativeComponent';
import NativeExpander, {
  type ExpanderChangeEvent,
} from './ExpanderNativeComponent';
import NativeInfoBadge from './InfoBadgeNativeComponent';
import NativeInfoBar, {
  type InfoBarCloseEvent,
} from './InfoBarNativeComponent';
import NativeNumberBox, {
  type NumberBoxChangeEvent,
} from './NumberBoxNativeComponent';
import NativeProgressBar from './ProgressBarNativeComponent';
import NativeRadioButton, {
  type RadioButtonChangeEvent,
} from './RadioButtonNativeComponent';
import NativeRatingControl, {
  type RatingChangeEvent,
} from './RatingControlNativeComponent';
import { useWinuiTheme, type WinuiTheme } from './theme';

type NativeEvent<T> = { nativeEvent: T };

type Shared = {
  theme?: WinuiTheme | null;
  disabled?: boolean;
};

export type { ComboBoxItem };

export type ComboBoxProps = ViewProps &
  Shared & {
    items: readonly ComboBoxItem[];
    selectedIndex?: number;
    placeholder?: string;
    ref?: Ref<ComponentRef<typeof NativeComboBox>>;
    onSelect?: (event: NativeEvent<ComboBoxChangeEvent>) => void;
    /** Fires with the index the user picked. The control stays on `selectedIndex` until this updates it. */
    onChange?: (index: number, item: ComboBoxItem | undefined) => void;
  };

export function ComboBox({
  ref,
  theme,
  onSelect,
  onChange,
  items,
  ...props
}: ComboBoxProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <NativeComboBox
      {...props}
      // The example app and the library resolve different copies of @types/react.
      ref={ref as never}
      items={items}
      theme={resolved}
      onSelect={(event) => {
        onSelect?.(event);
        const index = event.nativeEvent.itemIndex;
        onChange?.(index, items[index]);
      }}
    />
  );
}
ComboBox.displayName = 'ComboBox';

export type NumberBoxProps = ViewProps &
  Shared & {
    /** Omit, or pass null, to show an empty box. */
    value?: number | null;
    minimum?: number;
    maximum?: number;
    step?: number;
    spinButtons?: 'hidden' | 'compact' | 'inline';
    placeholder?: string;
    ref?: Ref<ComponentRef<typeof NativeNumberBox>>;
    onValueChange?: (event: NativeEvent<NumberBoxChangeEvent>) => void;
    /** `null` when the box is cleared. The control stays on `value` until this updates it. */
    onChange?: (value: number | null) => void;
  };

export function NumberBox({
  ref,
  theme,
  onValueChange,
  onChange,
  value,
  ...props
}: NumberBoxProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <NativeNumberBox
      {...props}
      ref={ref as never}
      value={value ?? undefined}
      theme={resolved}
      onValueChange={(event) => {
        onValueChange?.(event);
        onChange?.(event.nativeEvent.isEmpty ? null : event.nativeEvent.value);
      }}
    />
  );
}
NumberBox.displayName = 'NumberBox';

export type CheckBoxProps = ViewProps &
  Shared & {
    label?: string;
    checked?: boolean;
    indeterminate?: boolean;
    ref?: Ref<ComponentRef<typeof NativeCheckBox>>;
    onCheckedChange?: (event: NativeEvent<CheckBoxChangeEvent>) => void;
    /** The control stays on `checked` until this updates it. */
    onChange?: (checked: boolean, indeterminate: boolean) => void;
  };

export function CheckBox({
  ref,
  theme,
  onCheckedChange,
  onChange,
  ...props
}: CheckBoxProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <NativeCheckBox
      {...props}
      ref={ref as never}
      theme={resolved}
      onCheckedChange={(event) => {
        onCheckedChange?.(event);
        onChange?.(event.nativeEvent.checked, event.nativeEvent.indeterminate);
      }}
    />
  );
}
CheckBox.displayName = 'CheckBox';

export type RadioButtonProps = ViewProps &
  Shared & {
    label?: string;
    checked?: boolean;
    /**
     * WinUI group name. Each control is its own XAML island, so this does not
     * uncheck sibling radios. Use RadioGroup to manage a selection.
     */
    group?: string;
    ref?: Ref<ComponentRef<typeof NativeRadioButton>>;
    onCheckedChange?: (event: NativeEvent<RadioButtonChangeEvent>) => void;
    /** The control stays on `checked` until this updates it. */
    onChange?: (checked: boolean) => void;
  };

export function RadioButton({
  ref,
  theme,
  onCheckedChange,
  onChange,
  ...props
}: RadioButtonProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <NativeRadioButton
      {...props}
      ref={ref as never}
      theme={resolved}
      onCheckedChange={(event) => {
        onCheckedChange?.(event);
        onChange?.(event.nativeEvent.checked);
      }}
    />
  );
}
RadioButton.displayName = 'RadioButton';

export type RadioOption = {
  label: string;
  value: string;
  disabled?: boolean;
};

export type RadioGroupProps = {
  options: readonly RadioOption[];
  value?: string;
  onChange?: (value: string) => void;
  theme?: WinuiTheme | null;
  disabled?: boolean;
  style?: StyleProp<ViewStyle>;
};

/**
 * Owns the selected value for a set of radios. WinUI `group` does not cross
 * XAML islands, so each button is checked from this value instead.
 */
export function RadioGroup({
  options,
  value,
  onChange,
  theme,
  disabled,
  style,
}: RadioGroupProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <View style={style}>
      {options.map((option) => (
        <NativeRadioButton
          key={option.value}
          label={option.label}
          checked={option.value === value}
          disabled={disabled || option.disabled}
          theme={resolved}
          onCheckedChange={(event) => {
            if (event.nativeEvent.checked) {
              onChange?.(option.value);
            }
          }}
        />
      ))}
    </View>
  );
}
RadioGroup.displayName = 'RadioGroup';

export type RatingControlProps = ViewProps &
  Shared & {
    value?: number;
    maxRating?: number;
    ref?: Ref<ComponentRef<typeof NativeRatingControl>>;
    onValueChange?: (event: NativeEvent<RatingChangeEvent>) => void;
    /** The control stays on `value` until this updates it. */
    onChange?: (value: number) => void;
  };

export function RatingControl({
  ref,
  theme,
  onValueChange,
  onChange,
  ...props
}: RatingControlProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <NativeRatingControl
      {...props}
      ref={ref as never}
      theme={resolved}
      onValueChange={(event) => {
        onValueChange?.(event);
        onChange?.(event.nativeEvent.value);
      }}
    />
  );
}
RatingControl.displayName = 'RatingControl';

export type InfoBarProps = ViewProps &
  Shared & {
    title?: string;
    message?: string;
    severity?: 'informational' | 'success' | 'warning' | 'error';
    isOpen?: boolean;
    isClosable?: boolean;
    ref?: Ref<ComponentRef<typeof NativeInfoBar>>;
    /** Close was requested. `isOpen` stays unchanged until the parent sets it. */
    onClose?: (event: NativeEvent<InfoBarCloseEvent>) => void;
  };

export function InfoBar({ ref, theme, ...props }: InfoBarProps) {
  const resolved = useWinuiTheme(theme);
  return <NativeInfoBar {...props} ref={ref as never} theme={resolved} />;
}
InfoBar.displayName = 'InfoBar';

export type InfoBadgeProps = ViewProps &
  Shared & {
    /** Numeric value. -1 renders a dot with no count. */
    value?: number;
    ref?: Ref<ComponentRef<typeof NativeInfoBadge>>;
  };

export function InfoBadge({ ref, theme, ...props }: InfoBadgeProps) {
  const resolved = useWinuiTheme(theme);
  return <NativeInfoBadge {...props} ref={ref as never} theme={resolved} />;
}
InfoBadge.displayName = 'InfoBadge';

export type ProgressBarProps = ViewProps &
  Shared & {
    value?: number;
    minimum?: number;
    maximum?: number;
    isIndeterminate?: boolean;
    ref?: Ref<ComponentRef<typeof NativeProgressBar>>;
  };

export function ProgressBar({ ref, theme, ...props }: ProgressBarProps) {
  const resolved = useWinuiTheme(theme);
  return <NativeProgressBar {...props} ref={ref as never} theme={resolved} />;
}
ProgressBar.displayName = 'ProgressBar';

export type ExpanderProps = ViewProps &
  Shared & {
    header?: string;
    content?: string;
    isExpanded?: boolean;
    ref?: Ref<ComponentRef<typeof NativeExpander>>;
    onExpandChange?: (event: NativeEvent<ExpanderChangeEvent>) => void;
    /** The control stays on `isExpanded` until this updates it. */
    onChange?: (isExpanded: boolean) => void;
  };

export function Expander({
  ref,
  theme,
  onExpandChange,
  onChange,
  ...props
}: ExpanderProps) {
  const resolved = useWinuiTheme(theme);
  return (
    <NativeExpander
      {...props}
      ref={ref as never}
      theme={resolved}
      onExpandChange={(event) => {
        onExpandChange?.(event);
        onChange?.(event.nativeEvent.isExpanded);
      }}
    />
  );
}
Expander.displayName = 'Expander';
