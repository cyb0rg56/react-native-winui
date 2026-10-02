import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  Double,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type NumberBoxChangeEvent = Readonly<{
  value: Double;
}>;

export interface NumberBoxProps extends ViewProps {
  value?: WithDefault<Double, 0>;
  minimum?: Double;
  maximum?: Double;
  step?: Double;
  placeholder?: string;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onValueChange?: BubblingEventHandler<NumberBoxChangeEvent>;
}

export default codegenNativeComponent<NumberBoxProps>('NumberBox');
