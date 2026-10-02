#include "pch.h"

#include "XamlControl.h"

#include <cmath>
#include <limits>

#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Interop.h>

namespace winrt::Winui {

namespace {

struct MeasuringGuard {
  bool& measuring;
  explicit MeasuringGuard(bool& measuring) : measuring(measuring) {
    measuring = true;
  }
  ~MeasuringGuard() {
    measuring = false;
  }
};

bool SameSize(
    winrt::Windows::Foundation::Size const& left,
    winrt::Windows::Foundation::Size const& right) noexcept {
  return left.Width == right.Width && left.Height == right.Height;
}

void ApplyTheme(
    winrt::Microsoft::UI::Xaml::FrameworkElement const& element,
    std::string const& theme,
    winrt::Windows::UI::ViewManagement::AccessibilitySettings const& accessibility) {
  if (accessibility && accessibility.HighContrast()) {
    element.HighContrastAdjustment(winrt::Microsoft::UI::Xaml::ElementHighContrastAdjustment::Auto);
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Default);
    return;
  }

  if (theme == "dark") {
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Dark);
  } else if (theme == "light") {
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Light);
  } else {
    element.RequestedTheme(winrt::Microsoft::UI::Xaml::ElementTheme::Default);
  }
}

} // namespace

void InsertDouble(
    winrt::Microsoft::UI::Xaml::ResourceDictionary const& resources,
    wchar_t const* key,
    double value) {
  resources.Insert(winrt::box_value(winrt::hstring{key}), winrt::box_value(value));
}

void ApplyControlChrome(winrt::Microsoft::UI::Xaml::Controls::Control const& control) {
  using winrt::Microsoft::UI::Xaml::Controls::CheckBox;
  using winrt::Microsoft::UI::Xaml::Controls::ComboBox;
  using winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem;
  using winrt::Microsoft::UI::Xaml::Controls::InfoBadge;
  using winrt::Microsoft::UI::Xaml::Controls::ProgressBar;
  using winrt::Microsoft::UI::Xaml::Controls::RadioButton;
  using winrt::Microsoft::UI::Xaml::Controls::RatingControl;

  if (control.try_as<ProgressBar>()) {
    return;
  }

  if (auto const rating = control.try_as<RatingControl>()) {
    rating.FontSize(36);
    return;
  }

  if (auto const badge = control.try_as<InfoBadge>()) {
    badge.FontSize(16);
    badge.MinWidth(28);
    badge.MinHeight(28);
    badge.Height(28);
    return;
  }

  auto resources = winrt::Microsoft::UI::Xaml::ResourceDictionary{};
  InsertDouble(resources, L"ControlContentThemeFontSize", 24);
  InsertDouble(resources, L"BodyTextBlockFontSize", 24);

  if (control.try_as<CheckBox>() || control.try_as<RadioButton>()) {
    control.FontSize(24);
    control.VerticalContentAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Center);
    control.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MinHeightProperty());
  } else {
    // 24px type plus 16px of vertical padding fills the 56px row.
    InsertDouble(resources, L"TextControlThemeMinHeight", 56);
    resources.Insert(
        winrt::box_value(winrt::hstring{L"TextControlThemePadding"}),
        winrt::box_value(winrt::Microsoft::UI::Xaml::Thickness{12, 16, 10, 16}));

    if (control.try_as<ComboBox>()) {
      auto style = winrt::Microsoft::UI::Xaml::Style{winrt::xaml_typename<ComboBoxItem>()};
      style.Setters().Append(winrt::Microsoft::UI::Xaml::Setter{
          winrt::Microsoft::UI::Xaml::Controls::Control::FontSizeProperty(), winrt::box_value(14.0)});
      style.Setters().Append(winrt::Microsoft::UI::Xaml::Setter{
          winrt::Microsoft::UI::Xaml::FrameworkElement::MinHeightProperty(), winrt::box_value(32.0)});
      resources.Insert(winrt::box_value(winrt::xaml_typename<ComboBoxItem>()), style);
    }
  }

  control.Resources(resources);
}

namespace {

void ApplyAutomation(
    winrt::Microsoft::UI::Xaml::FrameworkElement const& element,
    winrt::hstring const& label,
    winrt::hstring const& testId) {
  if (label.empty()) {
    element.ClearValue(winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::NameProperty());
  } else {
    winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::SetName(element, label);
  }

  if (testId.empty()) {
    element.ClearValue(winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::AutomationIdProperty());
  } else {
    winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::SetAutomationId(element, testId);
  }
}

} // namespace

XamlIslandSession::XamlIslandSession() : m_alive(std::make_shared<bool>(true)) {}

XamlIslandSession::~XamlIslandSession() {
  if (m_alive) {
    *m_alive = false;
  }
  Close();
}

void XamlIslandSession::Attach(
    winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view,
    winrt::Microsoft::UI::Xaml::UIElement const& content) {
  Close();
  element = content.as<winrt::Microsoft::UI::Xaml::FrameworkElement>();
  // Canvas children are measured with the size we set, not the island slot.
  // Left/Top keeps that measure from being stretched by the parent.
  element.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Left);
  element.VerticalAlignment(winrt::Microsoft::UI::Xaml::VerticalAlignment::Top);

  host = winrt::Microsoft::UI::Xaml::Controls::Canvas{};
  host.Children().Append(element);

  island = winrt::Microsoft::UI::Xaml::XamlIsland{};
  island.Content(host);
  view.Connect(island.ContentIsland());
  dispatcher = element.DispatcherQueue();

  auto life = m_alive;
  destroyingRevoker = view.Destroying(
      winrt::auto_revoke,
      [this, life](
          winrt::Windows::Foundation::IInspectable const&,
          winrt::Microsoft::ReactNative::ComponentView const&) {
        if (life && *life) {
          Close();
        }
      });

  layoutRevoker = view.LayoutMetricsChanged(
      winrt::auto_revoke,
      [this, life](
          winrt::Windows::Foundation::IInspectable const&,
          winrt::Microsoft::ReactNative::LayoutMetricsChangedArgs const& args) {
        if (!life || !*life) {
          return;
        }
        auto const width = args.NewLayoutMetrics().Frame.Width;
        if (m_hasMeasured && std::abs(width - layoutWidth) < 0.5f) {
          return;
        }
        layoutWidth = width;
        Invalidate();
      });

  loadedRevoker = element.Loaded(winrt::auto_revoke, [this, life](auto const&, auto const&) {
    if (life && *life) {
      Invalidate();
    }
  });

  // HighContrastChanged fails with ERROR_NOT_FOUND (0x80070490) on this host.
  // ApplyTheme still reads HighContrast() whenever props are applied.
  accessibility = winrt::Windows::UI::ViewManagement::AccessibilitySettings{};
  try {
    contrastRevoker = accessibility.HighContrastChanged(
        winrt::auto_revoke, [this, life](auto const&, auto const&) {
          if (!life || !*life) {
            return;
          }
          if (dispatcher) {
            dispatcher.TryEnqueue([this, life] {
              if (life && *life) {
                ApplyRememberedTheme();
              }
            });
            return;
          }
          ApplyRememberedTheme();
        });
  } catch (winrt::hresult_error const&) {
    contrastRevoker.revoke();
  }
}

void XamlIslandSession::Close() noexcept {
  layoutRevoker.revoke();
  loadedRevoker.revoke();
  destroyingRevoker.revoke();
  contrastRevoker.revoke();
  element = nullptr;
  host = nullptr;
  state = nullptr;
  dispatcher = nullptr;
  accessibility = nullptr;
  layoutWidth = 0;
  m_hasMeasured = false;
  m_queued = false;
  m_remeasure = false;
  if (!island) {
    return;
  }

  auto closing = island;
  island = nullptr;
  closing.Content(nullptr);
  closing.Close();
}

void XamlIslandSession::SetState(winrt::Microsoft::ReactNative::IComponentState const& value) {
  state = value;
  Invalidate();
}

void XamlIslandSession::RememberTheme(std::string theme) {
  m_theme = std::move(theme);
  ApplyRememberedTheme();
}

void XamlIslandSession::ApplyRememberedTheme() {
  if (!element) {
    return;
  }
  auto settings = accessibility ? accessibility : winrt::Windows::UI::ViewManagement::AccessibilitySettings{};
  ApplyTheme(element, m_theme, settings);
}

void XamlIslandSession::Invalidate() {
  if (!element || !state || !state.Data() || !m_alive || !*m_alive) {
    return;
  }
  if (m_measuring) {
    m_remeasure = true;
    return;
  }
  if (m_queued) {
    return;
  }
  EnqueueMeasure();
}

void XamlIslandSession::EnqueueMeasure() {
  auto life = m_alive;
  auto run = [this, life] {
    if (!life || !*life) {
      return;
    }
    m_queued = false;
    MeasureNow();
  };

  if (dispatcher) {
    m_queued = true;
    if (dispatcher.TryEnqueue(run)) {
      return;
    }
    m_queued = false;
  }
  MeasureNow();
}

void XamlIslandSession::MeasureNow() {
  if (m_measuring || !element || !state || !state.Data()) {
    return;
  }

  {
  MeasuringGuard guard{m_measuring};

  auto const infinite = std::numeric_limits<float>::infinity();
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::WidthProperty());
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::HeightProperty());
  element.ClearValue(winrt::Microsoft::UI::Xaml::FrameworkElement::MaxWidthProperty());
  // The first pass creates template parts (rating stars, badge dots).
  // The second pass includes them in DesiredSize.
  element.Measure({infinite, infinite});
  element.Measure({infinite, infinite});
  auto natural = element.DesiredSize();
  natural.Width = std::ceil(natural.Width);
  natural.Height = std::ceil(natural.Height);

  bool const empty = !(natural.Width > 0.f) || !(natural.Height > 0.f);
  if (!(empty && !element.IsLoaded())) {
    winrt::Windows::Foundation::Size wrapped{0, 0};
    float wrappedWidth = 0;
    auto const slot = layoutWidth;
    if (!empty && slot > 1.f && std::isfinite(slot) && std::abs(slot - natural.Width) > 1.f) {
      auto const width = std::ceil(slot);
      element.Width(width);
      element.Measure({width, infinite});
      wrapped = element.DesiredSize();
      wrapped.Width = width;
      wrapped.Height = std::ceil(wrapped.Height);
      wrappedWidth = width;
    }

    auto const current = winrt::get_self<MeasuredSize>(state.Data());
    auto const unchanged = current && current->hasMeasured && SameSize(current->natural, natural) &&
        SameSize(current->wrapped, wrapped) && current->wrappedWidth == wrappedWidth;
    if (!unchanged) {
      state.UpdateStateWithMutation([natural, wrapped, wrappedWidth](winrt::Windows::Foundation::IInspectable const&) {
        auto measured = winrt::make_self<MeasuredSize>();
        measured->natural = natural;
        measured->wrapped = wrapped;
        measured->wrappedWidth = wrappedWidth;
        measured->hasMeasured = true;
        return measured.as<winrt::Windows::Foundation::IInspectable>();
      });
    }
    m_hasMeasured = true;
  }
  }

  if (m_remeasure && element && state) {
    m_remeasure = false;
    EnqueueMeasure();
  }
}

void GuardedCall(wchar_t const* name, std::function<void()> const& action) noexcept {
  try {
    action();
  } catch (winrt::hresult_error const& error) {
    auto message = std::wstring(L"react-native-winui ") + name + L": " + error.message().c_str() + L"\n";
    OutputDebugStringW(message.c_str());
  } catch (std::exception const& error) {
    auto message = std::wstring(L"react-native-winui ") + name + L": ";
    for (char const ch : std::string{error.what()}) {
      message.push_back(static_cast<wchar_t>(static_cast<unsigned char>(ch)));
    }
    message.push_back(L'\n');
    OutputDebugStringW(message.c_str());
  } catch (...) {
    auto message = std::wstring(L"react-native-winui ") + name + L": unknown exception\n";
    OutputDebugStringW(message.c_str());
  }
}

void PrepareElement(
    winrt::Microsoft::UI::Xaml::Controls::Control const& element,
    std::optional<std::string> const& theme,
    std::optional<bool> const& disabled,
    winrt::Microsoft::ReactNative::ViewProps const& viewProps,
    XamlIslandSession& session) {
  element.IsEnabled(!disabled.value_or(false));
  auto props = viewProps;
  ApplyAutomation(element, props.AccessibilityLabel(), props.TestId());
  session.RememberTheme(theme.value_or("system"));
}

void WrapTextBlocks(winrt::Microsoft::UI::Xaml::DependencyObject const& root) {
  if (!root) {
    return;
  }

  if (auto const text = root.try_as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>()) {
    text.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::Wrap);
  }

  auto const count = winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(root);
  for (int32_t index = 0; index < count; ++index) {
    WrapTextBlocks(winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetChild(root, index));
  }
}

winrt::hstring ToHString(std::string const& value) {
  return winrt::to_hstring(value);
}

winrt::hstring ToHString(std::optional<std::string> const& value) {
  return winrt::to_hstring(value.value_or(std::string{}));
}

void ConfigureXamlIsland(
    winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder,
    winrt::Windows::Foundation::Size fallback,
    std::function<void(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const&)> const&
        initialize) {
  auto viewBuilder = builder.as<winrt::Microsoft::ReactNative::IReactViewComponentBuilder>();
  viewBuilder.XamlSupport(true);

  builder.SetContentIslandComponentViewInitializer(
      [initialize](winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& islandView) noexcept {
        initialize(islandView);
      });

  viewBuilder.SetInitialStateDataFactory(
      [](winrt::Microsoft::ReactNative::IComponentProps const&) noexcept { return winrt::make<MeasuredSize>(); });

  viewBuilder.SetMeasureContentHandler(
      [fallback](
          winrt::Microsoft::ReactNative::ShadowNode const& shadowNode,
          winrt::Microsoft::ReactNative::LayoutContext const&,
          winrt::Microsoft::ReactNative::LayoutConstraints const& constraints) noexcept {
        auto const maxWidth = constraints.MaximumSize.Width;
        auto const minWidth = constraints.MinimumSize.Width;
        auto const widthExact = std::isfinite(maxWidth) && maxWidth > 0.f && std::abs(minWidth - maxWidth) < 0.5f;

        if (auto const data = shadowNode.StateData()) {
          auto const current = winrt::get_self<MeasuredSize>(data);
          if (current && current->hasMeasured) {
            if (!(current->natural.Width > 0.f) || !(current->natural.Height > 0.f)) {
              return current->natural;
            }
            if (widthExact && current->wrapped.Height > 0.f && std::abs(current->wrappedWidth - maxWidth) < 1.f) {
              return winrt::Windows::Foundation::Size{maxWidth, current->wrapped.Height};
            }
            if (widthExact) {
              return winrt::Windows::Foundation::Size{maxWidth, current->natural.Height};
            }
            if (std::isfinite(maxWidth) && maxWidth > 0.f && current->natural.Width > maxWidth &&
                current->wrapped.Height > 0.f && std::abs(current->wrappedWidth - maxWidth) < 1.f) {
              return current->wrapped;
            }
            return current->natural;
          }
        }
        return fallback;
      });
}

} // namespace winrt::Winui
