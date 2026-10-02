import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type ExpanderChangeEvent = Readonly<{
  isExpanded: boolean;
}>;

export interface ExpanderProps extends ViewProps {
  header?: string;
  content?: string;
  isExpanded?: WithDefault<boolean, false>;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onExpandChange?: BubblingEventHandler<ExpanderChangeEvent>;
}

export default codegenNativeComponent<ExpanderProps>('Expander');
