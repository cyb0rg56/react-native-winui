import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  Int32,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type ComboBoxItem = Readonly<{
  label: string;
  value?: string;
}>;

export type ComboBoxChangeEvent = Readonly<{
  value: string;
  itemIndex: Int32;
  text: string;
}>;

export interface ComboBoxProps extends ViewProps {
  items: ReadonlyArray<ComboBoxItem>;
  selectedIndex?: WithDefault<Int32, -1>;
  placeholder?: string;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onSelect?: BubblingEventHandler<ComboBoxChangeEvent>;
}

export default codegenNativeComponent<ComboBoxProps>('ComboBox');
