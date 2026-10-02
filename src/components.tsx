import type { ComponentType } from 'react';
import NativeCheckBox from './CheckBoxNativeComponent';
import NativeComboBox from './ComboBoxNativeComponent';
import NativeExpander from './ExpanderNativeComponent';
import NativeInfoBadge from './InfoBadgeNativeComponent';
import NativeInfoBar from './InfoBarNativeComponent';
import NativeNumberBox from './NumberBoxNativeComponent';
import NativeProgressBar from './ProgressBarNativeComponent';
import NativeRadioButton from './RadioButtonNativeComponent';
import NativeRatingControl from './RatingControlNativeComponent';
import { useWinuiTheme, type WinuiTheme } from './theme';

type Themed = {
  theme?: WinuiTheme | null;
};

function withWinuiTheme<P extends Themed>(
  NativeComponent: ComponentType<P>,
  displayName: string
): ComponentType<P> {
  function WinuiThemed(props: P) {
    const theme = useWinuiTheme(props.theme);
    return <NativeComponent {...props} theme={theme} />;
  }
  WinuiThemed.displayName = displayName;
  return WinuiThemed;
}

export const ComboBox = withWinuiTheme(NativeComboBox, 'ComboBox');
export const NumberBox = withWinuiTheme(NativeNumberBox, 'NumberBox');
export const CheckBox = withWinuiTheme(NativeCheckBox, 'CheckBox');
export const RadioButton = withWinuiTheme(NativeRadioButton, 'RadioButton');
export const RatingControl = withWinuiTheme(
  NativeRatingControl,
  'RatingControl'
);
export const InfoBar = withWinuiTheme(NativeInfoBar, 'InfoBar');
export const InfoBadge = withWinuiTheme(NativeInfoBadge, 'InfoBadge');
export const ProgressBar = withWinuiTheme(NativeProgressBar, 'ProgressBar');
export const Expander = withWinuiTheme(NativeExpander, 'Expander');
