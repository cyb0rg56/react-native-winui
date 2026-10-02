import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type RadioButtonChangeEvent = Readonly<{
  checked: boolean;
}>;

export interface RadioButtonProps extends ViewProps {
  label?: string;
  checked?: WithDefault<boolean, false>;
  group?: string;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onCheckedChange?: BubblingEventHandler<RadioButtonChangeEvent>;
}

export default codegenNativeComponent<RadioButtonProps>('RadioButton');
