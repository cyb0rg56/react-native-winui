# react-native-winui

Public npm library that maps WinUI 3 controls onto React Native Windows. Windows-only. New Architecture only.

Working package name: `react-native-winui`. The repo folder stays `rnw-winui3`.

## Why this library exists

React Native Windows already renders its own components (View, Text, Image, TextInput, ScrollView, Switch, and the rest of the core set) with the Windows App SDK composition layer. It does not expose the WinUI 3 control library.

That gap closed in React Native Windows 0.82. Community modules can host WinUI 3 XAML controls inside the Fabric tree, with layout, hit-testing, and accessibility. Before 0.82, wrapping controls such as Calendar, ComboBox, or NavigationView was limited to the Paper renderer.

Paper is gone as of 0.82. Every app on a current React Native Windows release runs Fabric. This library targets that baseline and does not ship a Paper implementation.

`react-native-xaml` (npm `react-native-xaml` 0.0.80) is a different product. It targets WinUI 2.7+ and React Native Windows >= 0.71, and its last published dev stack is React Native 0.79. It is not the implementation to extend.

## Platform baseline

Develop and publish against the current stable line:

| Piece | Version | Notes |
| --- | --- | --- |
| React Native | 0.84 | Matches the official library guide |
| react-native-windows | ^0.84.0 | Stable release, 30 Jun 2026. Fabric only |
| Library scaffold | `create-react-native-library@0.48.9` | The setup the React Native Windows team tests |
| Windows project | `cpp-lib` | `npx react-native init-windows --template cpp-lib --overwrite` |
| Toolchain | Visual Studio 2026 | Required to build React Native Windows 0.84 |
| Windows App SDK | 1.8 | 0.84's toolchain upgrade uses `Microsoft.WindowsAppSDK` 1.8.260508005. Use the package the targeted `react-native-windows` release pulls in |

Peer floor is `react-native-windows >= 0.82.0`, because that is the first release with community XAML hosting. Verify 0.82 and 0.83 before advertising them. `ContentIslandComponentView` and `XamlSupport` are marked experimental, so a version that compiles is not automatically a version that behaves.

React Native Windows 0.85 is preview (`0.85.0-preview.1`, Windows App SDK 1.8.260508005, .NET 10). Do not make the published peer a preview.

The framework itself is consumed as the prebuilt `Microsoft.ReactNative` NuGet package. This library ships C++ source. The app compiles it through autolinking.

## What the library ships

Typed React components that wrap `Microsoft.UI.Xaml.Controls`. Each control gets:

- A TypeScript spec named `<Name>NativeComponent.ts`, exported through `codegenNativeComponent`.
- A small JavaScript wrapper with a React-shaped API: controlled props, `on*` events, and `View` style for layout. Callers do not import the spec file.
- A C++/WinRT component view that hosts one WinUI 3 control.

Leave these to React Native Windows: View, Text, Image, TextInput, ScrollView, Pressable, Switch, ActivityIndicator, Button, and Modal. Paper-only Flyout and Popup were not brought forward as their own Fabric components; Modal is the framework replacement. WebView2 is out of scope.

## Architecture

```
JS wrapper
  -> *NativeComponent.ts spec
    -> codegen-windows (componentsWindows)
      -> C++ Register*NativeComponent
        -> XamlSupport(true)
        -> ContentIslandComponentView
          -> XamlIsland.Content = WinUI control
          -> islandView.Connect(island.ContentIsland())
```

`XamlSupport(true)` is the opt-in that loads `XamlApplication`. React Native Windows lazy-loads it only when a registered component asks for it, because `WindowsXamlManager.InitializeForCurrentThread()` is expensive. Apps that never import this package do not pay that cost. Apps that import one control do.

The public `XamlHost` component was intentionally left out of 0.82. Third-party modules host XAML through `ContentIslandComponentView`, which is the supported path.

Official reference implementation: `PickerXaml` in `microsoft/react-native-windows` (`packages/sample-custom-component`). It wraps `Microsoft.UI.Xaml.Controls.ComboBox`.

### Per-control contract

1. **Spec.** Props extend `ViewProps`. Use codegen types (`Int32`, `WithDefault`, `ReadonlyArray`, `BubblingEventHandler`). The file name must end in `NativeComponent.ts` or codegen ignores it.
2. **Registration.** There is no attribute-based auto-registration for view components. Call the generated `Register*NativeComponent` from `ReactPackageProvider::CreatePackage`, and add the `.h` / `.cpp` to the `vcxproj`.
3. **Island.** Create the WinUI control, assign it to `XamlIsland.Content`, then `Connect` the island. Store the island view as a `weak_ref`.
4. **Props.** `UpdateProps` writes WinUI properties. Suspend native event handlers while applying props so a prop update does not echo back as a user event.
5. **Events.** WinRT revokers (`auto_revoke`) forward control events through the generated event emitter. Use `get_weak()` in every handler.
6. **Layout.** Yoga does not know the control's desired size. Measure the control, store the size in component state, and return it from `SetMeasureContentHandler`. Keep an explicit `style` width and height as the fallback before the first measure (the ComboBox sample falls back to 100×32).
7. **Teardown.** Close and drop the island when the component view is destroyed. A leaked island breaks window input routing.
8. **Accessibility.** Let the WinUI control's automation peer speak. React Native Windows 0.82 added an accessibility override for third-party components. 0.83 fixed Narrator focus crossing into XAML islands hosted by community modules, and `ChildSite.AutomationOption` can be set when the default peer is wrong.
9. **Theme.** WinUI follows the XAML application theme. Keep the hosted controls aligned with the React Native color scheme and high contrast. React Native Windows 0.84 already makes default text color theme-aware; hosted controls still need an explicit theme hook.

Composition-only visuals (`Microsoft.UI.Composition`, as in the docs' `CircleMask` sample) are the wrong tool for a control that already exists in WinUI. Use them only when a control cannot be hosted as XAML.

## Codegen

`package.json` must generate component headers, not only module headers:

```json
"codegenConfig": {
  "name": "RNWinuiSpec",
  "type": "all",
  "jsSrcsDir": "src",
  "includesGeneratedCode": true,
  "windows": {
    "namespace": "winuiCodegen",
    "generators": ["modulesWindows", "componentsWindows"],
    "outputDirectory": "windows/<project>/codegen",
    "separateDataTypes": true
  }
}
```

`react-native codegen-windows` runs at the start of every native build. Commit the generated headers so consumers are not blocked when codegen is skipped, and treat the spec as the source of truth when they drift.

## Scaffold

From the official getting-started guide, which is written for 0.84:

```bat
npx --yes create-react-native-library@0.48.9 <name> --react-native-version ^0.84.0
```

Answers the guide expects: library type `turbo-module` (`--type turbo-module`), languages Kotlin and Objective-C (`--languages kotlin-objc`). Windows is added after that, not by the scaffold.

```bat
yarn add react-native-windows@^0.84.0 --dev
yarn add react-native-windows@* --peer
npx react-native init-windows --template cpp-lib --overwrite
```

`init-windows` also initializes the `example` app. Run it with `yarn example react-native run-windows` from the library root.

## Control map

Ship leaf controls first. Each one is a single `UIElement`, a controlled value, and a bubbling event. Shell controls come later because they own popups, focus, and light-dismiss.

### First set

| React component | WinUI type | Why first |
| --- | --- | --- |
| `ComboBox` | `ComboBox` | Official `PickerXaml` sample. Proves the island, measure, and event path |
| `NumberBox` | `NumberBox` | Numeric input React Native does not have |
| `CheckBox` | `CheckBox` | |
| `RadioButton` | `RadioButton` | Grouped by a `name` or parent prop |
| `RatingControl` | `RatingControl` | |
| `InfoBar` | `InfoBar` | Inline status. Controlled `isOpen` |
| `InfoBadge` | `InfoBadge` | |
| `Expander` | `Expander` | Header plus children needs a child-mount story; do it after the leaves if children are hard |
| `ProgressBar` | `ProgressBar` | Determinate bar. `ActivityIndicator` already covers the ring |

`ComboBox` is the template. The next controls copy its registration, measure state, and event-suspend pattern.

### Later

`CalendarDatePicker`, `CalendarView`, `DatePicker`, `TimePicker`, `AutoSuggestBox`, `ColorPicker`, `DropDownButton`, `SplitButton`, `PersonPicture`, `BreadcrumbBar`, `PagerControl`, `TeachingTip`, `TabView`, `TreeView`, `NavigationView`, `MenuBar`, `CommandBar`, `ContentDialog`, `TitleBar`.

`NavigationView`, `TeachingTip`, and `ContentDialog` wait until island focus, light-dismiss, and popup layering are proven on a simple control.

## Public package

- MIT license.
- `peerDependencies`: `react`, `react-native`, `react-native-windows >= 0.82.0`.
- `devDependencies`: the 0.84 line the example app builds with.
- `files` includes `src`, the built JS, and `windows/**` sources (vcxproj, C++, IDL, codegen headers). Excludes `example`, build output, and tests.
- `react-native.config.js` from the `cpp-lib` template so autolinking finds the Windows project.
- TypeScript types published with the package. Props and event payloads are the public API; the C++ types are not.
- README states the peer floor, the Visual Studio 2026 requirement, and that importing any control initializes WinUI for the process.

## Example app

The scaffolded `example` app is the test harness. Every shipped control has a screen that:

- Renders with an explicit size and with measure-driven size.
- Round-trips a controlled value (prop in, event out, prop back).
- Survives mount, unmount, and remount without breaking input on the rest of the window.
- Follows light, dark, and high contrast.
- Is reachable with keyboard and Narrator.

## Risks

- `IReactCompositionViewComponentBuilder.SetContentIslandComponentViewInitializer` and `ContentIslandComponentView.Connect` are experimental. Track React Native Windows releases and retest before bumping the peer.
- The first import of any control initializes XAML on the UI thread. Document that, and do not initialize it from a module that might be imported unused.
- Desired-size measurement is asynchronous relative to the first Yoga pass. Components must tolerate one frame at the fallback size.
- Popup controls (ComboBox dropdown, DatePicker, TeachingTip, ContentDialog) can escape the island's clip and focus scope. Prove dropdowns on `ComboBox` before taking on `NavigationView`.
- `XamlSupport` still lives on the view-component builder, and moving it is an open framework issue. Call it in the registration callback, which is what the sample does.

## References

- [Native Platform: Getting Started](https://microsoft.github.io/react-native-windows/docs/native-platform-getting-started) — library scaffold for 0.84, `cpp-lib` template
- [Native Platform: Native Components](https://microsoft.github.io/react-native-windows/docs/native-platform-components/) — spec, codegen, C++ view, package registration
- [codegen-windows](https://microsoft.github.io/react-native-windows/docs/codegen-windows-cli/) — `codegenConfig.windows`
- [init-windows](https://microsoft.github.io/react-native-windows/docs/init-windows-cli/) — `cpp-lib` is the library template
- [React Native Windows 0.82](https://devblogs.microsoft.com/react-native/%F0%9F%9A%80react-native-windows-v0-82-is-here/) — Fabric only, community XAML hosting
- [React Native Windows 0.83](https://devblogs.microsoft.com/react-native/%F0%9F%9A%80react-native-windows-v0-83-is-here/) — Narrator focus into third-party XAML islands
- [React Native Windows 0.84](https://devblogs.microsoft.com/react-native/react-native-windows-v0-84-is-here/) — current stable, Visual Studio 2026
- [ContentIslandComponentView](https://github.com/microsoft/react-native-windows-samples/blob/main/docs/native-api/ContentIslandComponentView-api-windows.md) — experimental `Connect`
- [Xaml on demand (PR 15407)](https://github.com/microsoft/react-native-windows/pull/15407) — `XamlSupport`, and why `XamlHost` was not published
- `PickerXaml` sample — `packages/sample-custom-component` in `microsoft/react-native-windows` (commit `5b9be55`)
- [WinUI 3 controls](https://learn.microsoft.com/en-us/windows/windows-app-sdk/api/winrt/microsoft.ui.xaml.controls?view=windows-app-sdk-2.0) — `Microsoft.UI.Xaml.Controls`
