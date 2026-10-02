import type { ViewProps } from 'react-native';
import type {
  Double,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export interface ProgressBarProps extends ViewProps {
  /** Progress from `minimum` to `maximum`. Ignored while isIndeterminate is true. */
  value?: WithDefault<Double, 0>;
  minimum?: WithDefault<Double, 0>;
  maximum?: WithDefault<Double, 100>;
  isIndeterminate?: WithDefault<boolean, false>;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
}

export default codegenNativeComponent<ProgressBarProps>('ProgressBar');
