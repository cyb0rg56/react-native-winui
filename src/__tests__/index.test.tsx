import TestRenderer from 'react-test-renderer';
import type { ReactNode } from 'react';
import {
  CheckBox,
  ComboBox,
  Expander,
  InfoBadge,
  InfoBar,
  NumberBox,
  ProgressBar,
  RadioButton,
  RadioGroup,
  RatingControl,
} from '../components';
import { resolveWinuiTheme } from '../theme';

jest.mock('react-native', () => ({
  useColorScheme: () => 'light',
  View: ({ children }: { children?: ReactNode }) => children ?? null,
}));

function mockNative(name: string) {
  const React = require('react');
  function Native(props: Record<string, unknown>) {
    return React.createElement(name, { ...props, testID: `native-${name}` });
  }
  return { __esModule: true, default: Native };
}

jest.mock('../CheckBoxNativeComponent', () => mockNative('CheckBox'));
jest.mock('../ComboBoxNativeComponent', () => mockNative('ComboBox'));
jest.mock('../ExpanderNativeComponent', () => mockNative('Expander'));
jest.mock('../InfoBadgeNativeComponent', () => mockNative('InfoBadge'));
jest.mock('../InfoBarNativeComponent', () => mockNative('InfoBar'));
jest.mock('../NumberBoxNativeComponent', () => mockNative('NumberBox'));
jest.mock('../ProgressBarNativeComponent', () => mockNative('ProgressBar'));
jest.mock('../RadioButtonNativeComponent', () => mockNative('RadioButton'));
jest.mock('../RatingControlNativeComponent', () => mockNative('RatingControl'));

function render(node: React.ReactElement) {
  let renderer: TestRenderer.ReactTestRenderer;
  TestRenderer.act(() => {
    renderer = TestRenderer.create(node);
  });
  return renderer!;
}

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

describe('themed controls', () => {
  it.each([
    ['CheckBox', <CheckBox />],
    ['ComboBox', <ComboBox items={[]} />],
    ['Expander', <Expander />],
    ['InfoBadge', <InfoBadge />],
    ['InfoBar', <InfoBar />],
    ['NumberBox', <NumberBox />],
    ['ProgressBar', <ProgressBar />],
    ['RadioButton', <RadioButton />],
    ['RatingControl', <RatingControl />],
  ] as const)('%s passes the resolved theme through', (name, element) => {
    const renderer = render(element);
    expect(
      renderer.root.findByProps({ testID: `native-${name}` }).props.theme
    ).toBe('light');
  });

  it('does not replace an explicit theme', () => {
    const renderer = render(<ComboBox items={[]} theme="system" />);
    expect(
      renderer.root.findByProps({ testID: 'native-ComboBox' }).props.theme
    ).toBe('system');
  });
});

describe('controlled callbacks', () => {
  it('reports the combo selection and leaves committing it to onChange', () => {
    const items = [{ label: 'C++', value: 'cpp' }];
    const onChange = jest.fn();
    const onSelect = jest.fn();
    const renderer = render(
      <ComboBox
        items={items}
        selectedIndex={-1}
        onChange={onChange}
        onSelect={onSelect}
      />
    );
    const native = renderer.root.findByProps({ testID: 'native-ComboBox' });
    native.props.onSelect({
      nativeEvent: { itemIndex: 0, text: 'C++', value: 'cpp' },
    });
    expect(onSelect).toHaveBeenCalledTimes(1);
    expect(onChange).toHaveBeenCalledWith(0, items[0]);
    expect(native.props.selectedIndex).toBe(-1);
  });

  it('reports an empty number box as null', () => {
    const onChange = jest.fn();
    const renderer = render(<NumberBox value={4} onChange={onChange} />);
    renderer.root
      .findByProps({ testID: 'native-NumberBox' })
      .props.onValueChange({
        nativeEvent: { value: 0, isEmpty: true },
      });
    expect(onChange).toHaveBeenCalledWith(null);
  });

  it('reports a checkbox change without writing the prop', () => {
    const onChange = jest.fn();
    const renderer = render(<CheckBox checked={false} onChange={onChange} />);
    const native = renderer.root.findByProps({ testID: 'native-CheckBox' });
    native.props.onCheckedChange({
      nativeEvent: { checked: true, indeterminate: false },
    });
    expect(onChange).toHaveBeenCalledWith(true, false);
    expect(native.props.checked).toBe(false);
  });
});

describe('RadioGroup', () => {
  it('checks only the selected option and reports that value', () => {
    const onChange = jest.fn();
    const renderer = render(
      <RadioGroup
        value="top"
        onChange={onChange}
        options={[
          { label: 'Left', value: 'left' },
          { label: 'Top', value: 'top' },
        ]}
      />
    );
    const radios = renderer.root.findAllByProps({
      testID: 'native-RadioButton',
    });
    expect(radios.map((radio) => radio.props.checked)).toEqual([false, true]);
    radios[0]?.props.onCheckedChange({ nativeEvent: { checked: true } });
    expect(onChange).toHaveBeenCalledWith('left');
  });
});
