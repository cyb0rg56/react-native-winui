import type { ViewProps } from 'react-native';
import type {
  Int32,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export interface InfoBadgeProps extends ViewProps {
  /** Numeric value. -1 renders a dot with no count. */
  value?: WithDefault<Int32, -1>;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
}

export default codegenNativeComponent<InfoBadgeProps>('InfoBadge');
