import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  Double,
  Int32,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type RatingChangeEvent = Readonly<{
  value: Double;
}>;

export interface RatingControlProps extends ViewProps {
  value?: WithDefault<Double, 0>;
  maxRating?: WithDefault<Int32, 5>;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onValueChange?: BubblingEventHandler<RatingChangeEvent>;
}

export default codegenNativeComponent<RatingControlProps>('RatingControl');
