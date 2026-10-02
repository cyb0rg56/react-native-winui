import TestRenderer from 'react-test-renderer';
import { ComboBox } from '../components';
import { resolveWinuiTheme } from '../theme';

jest.mock('react-native', () => ({
  useColorScheme: () => 'light',
}));

jest.mock('../ComboBoxNativeComponent', () => {
  const React = require('react');
  function NativeComboBox(props: Record<string, unknown>) {
    return React.createElement('NativeComboBox', {
      ...props,
      testID: 'native-combobox',
    });
  }
  return { __esModule: true, default: NativeComboBox };
});

describe('resolveWinuiTheme', () => {
  it('follows the React Native color scheme when theme is omitted', () => {
    expect(resolveWinuiTheme(undefined, 'dark')).toBe('dark');
    expect(resolveWinuiTheme(undefined, 'light')).toBe('light');
    expect(resolveWinuiTheme(undefined, null)).toBe('light');
  });

  it('keeps an explicit theme, including system', () => {
    expect(resolveWinuiTheme('system', 'dark')).toBe('system');
    expect(resolveWinuiTheme('light', 'dark')).toBe('light');
  });
});

describe('ComboBox', () => {
  it('passes items and the resolved theme to the native component', () => {
    const items = [{ label: 'C++', value: 'cpp' }];
    let renderer: TestRenderer.ReactTestRenderer;
    TestRenderer.act(() => {
      renderer = TestRenderer.create(
        <ComboBox items={items} selectedIndex={0} />
      );
    });
    const native = renderer!.root.findByProps({ testID: 'native-combobox' });
    expect(native.props.items).toBe(items);
    expect(native.props.selectedIndex).toBe(0);
    expect(native.props.theme).toBe('light');
  });

  it('does not replace an explicit theme', () => {
    let renderer: TestRenderer.ReactTestRenderer;
    TestRenderer.act(() => {
      renderer = TestRenderer.create(<ComboBox items={[]} theme="system" />);
    });
    expect(
      renderer!.root.findByProps({ testID: 'native-combobox' }).props.theme
    ).toBe('system');
  });
});
