import type { ViewProps } from 'react-native';
import type {
  BubblingEventHandler,
  WithDefault,
} from 'react-native/Libraries/Types/CodegenTypes';
import codegenNativeComponent from 'react-native/Libraries/Utilities/codegenNativeComponent';

export type InfoBarCloseEvent = Readonly<{
  isOpen: boolean;
}>;

export interface InfoBarProps extends ViewProps {
  title?: string;
  message?: string;
  severity?: WithDefault<
    'informational' | 'success' | 'warning' | 'error',
    'informational'
  >;
  isOpen?: WithDefault<boolean, true>;
  isClosable?: WithDefault<boolean, true>;
  disabled?: WithDefault<boolean, false>;
  theme?: WithDefault<'light' | 'dark' | 'system', 'system'>;
  onClose?: BubblingEventHandler<InfoBarCloseEvent>;
}

export default codegenNativeComponent<InfoBarProps>('InfoBar');
