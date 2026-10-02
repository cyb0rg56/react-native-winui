import { useState } from 'react';
import {
  Pressable,
  ScrollView,
  StyleSheet,
  Text,
  useColorScheme,
  View,
} from 'react-native';
import {
  CheckBox,
  ComboBox,
  Expander,
  InfoBadge,
  InfoBar,
  NumberBox,
  ProgressBar,
  RadioButton,
  RatingControl,
  resolveWinuiTheme,
  type WinuiTheme,
} from 'react-native-winui';

const comboItems = [
  { label: 'C++', value: 'cpp' },
  { label: 'TypeScript', value: 'ts' },
  { label: 'C#', value: 'cs' },
];

const themes: WinuiTheme[] = ['light', 'dark', 'system'];

const palette = {
  light: {
    page: '#F3F3F3',
    ink: '#1A1A1A',
    muted: '#5C5C5C',
    chip: '#FFFFFF',
    line: '#C4C4C4',
  },
  dark: {
    page: '#1F1F1F',
    ink: '#FFFFFF',
    muted: '#C8C8C8',
    chip: '#2B2B2B',
    line: '#5A5A5A',
  },
} as const;

export default function App() {
  const scheme = useColorScheme();
  const [mountKey, setMountKey] = useState(0);
  const [theme, setTheme] = useState<WinuiTheme | undefined>(undefined);
  const [comboIndex, setComboIndex] = useState(2);
  const [number, setNumber] = useState(4);
  const [checked, setChecked] = useState(false);
  const [radio, setRadio] = useState('left');
  const [rating, setRating] = useState(3);
  const [infoOpen, setInfoOpen] = useState(true);
  const [expanded, setExpanded] = useState(true);
  const [progress, setProgress] = useState(40);

  const active = resolveWinuiTheme(theme, scheme);
  const colors =
    palette[
      active === 'dark' || (active === 'system' && scheme === 'dark')
        ? 'dark'
        : 'light'
    ];

  return (
    <ScrollView
      style={{ backgroundColor: colors.page }}
      contentContainerStyle={styles.content}
    >
      <Text
        style={[
          styles.title,
          { color: colors.ink, backgroundColor: colors.page },
        ]}
      >
        react-native-winui
      </Text>
      <Text
        style={[
          styles.note,
          { color: colors.muted, backgroundColor: colors.page },
        ]}
      >
        WinUI 3 controls hosted in the Fabric tree. Pick a theme or leave it on
        the React Native color scheme.
      </Text>

      <View style={styles.row}>
        {themes.map((item) => {
          const selected = theme === item;
          return (
            <Pressable
              key={item}
              accessibilityRole="button"
              onPress={() => setTheme(item)}
              style={[
                styles.chip,
                { backgroundColor: colors.chip, borderColor: colors.line },
                selected && { borderColor: colors.ink },
              ]}
            >
              <Text style={{ color: colors.ink, backgroundColor: colors.chip }}>
                {item}
              </Text>
            </Pressable>
          );
        })}
        <Pressable
          accessibilityRole="button"
          onPress={() => setMountKey((value) => value + 1)}
          style={[
            styles.chip,
            { backgroundColor: colors.chip, borderColor: colors.line },
          ]}
        >
          <Text style={{ color: colors.ink, backgroundColor: colors.chip }}>
            Remount
          </Text>
        </Pressable>
      </View>

      <View key={mountKey} style={styles.gallery}>
        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          ComboBox
        </Text>
        <ComboBox
          theme={theme}
          items={comboItems}
          selectedIndex={comboIndex}
          placeholder="Language"
          onSelect={(event) => setComboIndex(event.nativeEvent.itemIndex)}
          style={styles.field}
        />
        <Text
          style={[
            styles.value,
            { color: colors.muted, backgroundColor: colors.page },
          ]}
        >
          Selected: {comboItems[comboIndex]?.label ?? 'none'}
        </Text>

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          NumberBox
        </Text>
        <NumberBox
          theme={theme}
          value={number}
          minimum={0}
          maximum={10}
          step={1}
          onValueChange={(event) => setNumber(event.nativeEvent.value)}
          style={styles.field}
        />

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          CheckBox
        </Text>
        <CheckBox
          theme={theme}
          label="Build with Fabric"
          checked={checked}
          onCheckedChange={(event) => setChecked(event.nativeEvent.checked)}
          style={styles.hug}
        />

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          RadioButton
        </Text>
        {(['left', 'top', 'hidden'] as const).map((value) => (
          <RadioButton
            key={value}
            theme={theme}
            label={value}
            group="pane"
            checked={radio === value}
            onCheckedChange={(event) => {
              if (event.nativeEvent.checked) {
                setRadio(value);
              }
            }}
            style={styles.hug}
          />
        ))}

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          RatingControl
        </Text>
        <RatingControl
          theme={theme}
          value={rating}
          maxRating={5}
          onValueChange={(event) => setRating(event.nativeEvent.value)}
          style={styles.hug}
        />

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          InfoBar
        </Text>
        <InfoBar
          theme={theme}
          title="Package ready"
          message="WinUI 3 controls are hosted in the Fabric tree."
          severity="success"
          isOpen={infoOpen}
          onClose={() => setInfoOpen(false)}
          style={styles.field}
        />
        {!infoOpen ? (
          <Pressable
            onPress={() => setInfoOpen(true)}
            style={[
              styles.chip,
              { backgroundColor: colors.chip, borderColor: colors.line },
            ]}
          >
            <Text style={{ color: colors.ink, backgroundColor: colors.chip }}>
              Show InfoBar
            </Text>
          </Pressable>
        ) : null}

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          InfoBadge
        </Text>
        <View style={styles.row}>
          <InfoBadge theme={theme} value={3} style={styles.badge} />
          <InfoBadge theme={theme} value={-1} style={styles.badge} />
        </View>

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          ProgressBar
        </Text>
        <ProgressBar theme={theme} value={progress} style={styles.progress} />
        <ProgressBar theme={theme} isIndeterminate style={styles.progress} />
        <View style={styles.row}>
          <Pressable
            onPress={() => setProgress((value) => Math.max(0, value - 10))}
            style={[
              styles.chip,
              { backgroundColor: colors.chip, borderColor: colors.line },
            ]}
          >
            <Text style={{ color: colors.ink, backgroundColor: colors.chip }}>
              -10
            </Text>
          </Pressable>
          <Pressable
            onPress={() => setProgress((value) => Math.min(100, value + 10))}
            style={[
              styles.chip,
              { backgroundColor: colors.chip, borderColor: colors.line },
            ]}
          >
            <Text style={{ color: colors.ink, backgroundColor: colors.chip }}>
              +10
            </Text>
          </Pressable>
        </View>

        <Text
          style={[
            styles.label,
            { color: colors.ink, backgroundColor: colors.page },
          ]}
        >
          Expander
        </Text>
        <Expander
          theme={theme}
          header="Build notes"
          content="Expander body is a WinUI string. React children are not mounted inside the island."
          isExpanded={expanded}
          onExpandChange={(event) => setExpanded(event.nativeEvent.isExpanded)}
          style={styles.field}
        />
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  content: {
    padding: 24,
    gap: 12,
  },
  title: {
    fontSize: 24,
    fontWeight: '600',
  },
  note: {
    fontSize: 14,
    lineHeight: 20,
  },
  gallery: {
    gap: 8,
    maxWidth: 420,
  },
  label: {
    marginTop: 8,
    fontSize: 16,
    fontWeight: '600',
  },
  value: {
    fontSize: 14,
  },
  row: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    gap: 8,
    alignItems: 'center',
  },
  chip: {
    paddingHorizontal: 12,
    paddingVertical: 8,
    borderWidth: 1,
    borderRadius: 6,
  },
  field: {
    width: 360,
    alignSelf: 'flex-start',
  },
  hug: {
    alignSelf: 'flex-start',
  },
  badge: {
    width: 28,
    height: 28,
  },
  progress: {
    width: 360,
    height: 16,
    alignSelf: 'flex-start',
  },
});
