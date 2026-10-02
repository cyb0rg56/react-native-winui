import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type CheckBoxChangeEvent = Readonly<{
  checked: boolean;
  indeterminate: boolean;
}>;

export interface CheckBoxProps extends ViewProps {
  label?: string;
  checked?: WithDefault<boolean, false>;
  indeterminate?: WithDefault<boolean, false>;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onCheckedChange?: BubblingEventHandler<CheckBoxChangeEvent>;
}

export default codegenNativeComponent<CheckBoxProps>('CheckBox');
