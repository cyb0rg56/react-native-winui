import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  Double,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type NumberBoxChangeEvent = Readonly<{
  value: Double;
  /** True when the box is cleared. `value` is 0 in that case. */
  isEmpty: boolean;
}>;

export interface NumberBoxProps extends ViewProps {
  /** Omit to show an empty box. */
  value?: Double;
  minimum?: Double;
  maximum?: Double;
  step?: Double;
  spinButtons?: WithDefault<'hidden' | 'compact' | 'inline', 'compact'>;
  placeholder?: string;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onValueChange?: BubblingEventHandler<NumberBoxChangeEvent>;
}

export default codegenNativeComponent<NumberBoxProps>('NumberBox');
