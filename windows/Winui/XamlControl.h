#pragma once

#include "pch.h"

#include <functional>
#include <memory>
#include <string>

#include <winrt/Microsoft.ReactNative.Composition.h>
#include <winrt/Microsoft.ReactNative.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.ViewManagement.h>

namespace winrt::Winui {

struct MeasuredSize : winrt::implements<MeasuredSize, winrt::IInspectable> {
  MeasuredSize() = default;
  winrt::Windows::Foundation::Size natural{0, 0};
  winrt::Windows::Foundation::Size wrapped{0, 0};
  float wrappedWidth{0};
  // False until a loaded control has been measured, including a real 0 size
  // for a collapsed control. The measure handler uses the fallback until then.
  bool hasMeasured{false};
};

// Hosts one WinUI element in a XAML island.
//
// Callbacks capture this. That is safe only while the session stays put:
// copy and move are deleted, and queued dispatcher work checks m_alive, which
// the destructor clears before members are torn down.
struct XamlIslandSession {
  XamlIslandSession();
  ~XamlIslandSession();
  XamlIslandSession(XamlIslandSession const&) = delete;
  XamlIslandSession& operator=(XamlIslandSession const&) = delete;
  XamlIslandSession(XamlIslandSession&&) = delete;
  XamlIslandSession& operator=(XamlIslandSession&&) = delete;

  void Attach(
      winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view,
      winrt::Microsoft::UI::Xaml::UIElement const& content);
  void Close() noexcept;
  void SetState(winrt::Microsoft::ReactNative::IComponentState const& value);
  void Invalidate();
  void RememberTheme(std::string theme);

  winrt::Microsoft::UI::Xaml::FrameworkElement element{nullptr};

 private:
  void MeasureNow();
  void EnqueueMeasure();
  void ApplyRememberedTheme();

  std::shared_ptr<bool> m_alive;
  winrt::Microsoft::UI::Xaml::XamlIsland island{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::Canvas host{nullptr};
  winrt::Microsoft::ReactNative::IComponentState state{nullptr};
  winrt::Microsoft::UI::Dispatching::DispatcherQueue dispatcher{nullptr};
  winrt::Windows::UI::ViewManagement::AccessibilitySettings accessibility{nullptr};
  std::string m_theme{"system"};
  float layoutWidth{0};
  bool m_hasMeasured{false};
  bool m_measuring{false};
  bool m_queued{false};
  bool m_remeasure{false};
  winrt::Microsoft::ReactNative::ComponentView::Destroying_revoker destroyingRevoker;
  winrt::Microsoft::ReactNative::ComponentView::LayoutMetricsChanged_revoker layoutRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::Loaded_revoker loadedRevoker;
  winrt::Windows::UI::ViewManagement::AccessibilitySettings::HighContrastChanged_revoker contrastRevoker;
};

// Logs WinRT and standard exceptions. Callers are noexcept; an uncaught
// hresult_error would terminate the process.
void GuardedCall(wchar_t const* name, std::function<void()> const& action) noexcept;

// Overrides WinUI theme resources so text controls use a 24px / 56px row.
// Call before the control is added to the island.
void ApplyControlChrome(winrt::Microsoft::UI::Xaml::Controls::Control const& control);

void PrepareElement(
    winrt::Microsoft::UI::Xaml::Controls::Control const& element,
    std::optional<std::string> const& theme,
    std::optional<bool> const& disabled,
    winrt::Microsoft::ReactNative::ViewProps const& viewProps,
    XamlIslandSession& session);

void WrapTextBlocks(winrt::Microsoft::UI::Xaml::DependencyObject const& root);

winrt::hstring ToHString(std::optional<std::string> const& value);
winrt::hstring ToHString(std::string const& value);

template <typename TProps>
bool ChromeChanged(winrt::com_ptr<TProps> const& next, winrt::com_ptr<TProps> const& previous) {
  if (!next) {
    return false;
  }
  if (!previous) {
    return true;
  }
  if (previous->theme != next->theme || previous->disabled != next->disabled) {
    return true;
  }

  auto nextView = next->ViewProps;
  auto previousView = previous->ViewProps;
  return nextView.AccessibilityLabel() != previousView.AccessibilityLabel() ||
      nextView.TestId() != previousView.TestId();
}

void ConfigureXamlIsland(
    winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder,
    winrt::Windows::Foundation::Size fallback,
    std::function<void(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const&)> const&
        initialize);

// Shared attach, measure, and event-suspend path for a hosted control.
// TSelf is the winrt::implements type. TBase is the codegen Base* class.
// TControl is the WinUI control type.
template <typename TSelf, typename TBase, typename TControl>
struct XamlComponentView : TBase {
  ~XamlComponentView() {
    m_session.Close();
  }

  void UpdateState(
      winrt::Microsoft::ReactNative::ComponentView const&,
      winrt::Microsoft::ReactNative::IComponentState const& newState) noexcept override {
    GuardedCall(L"UpdateState", [&] { m_session.SetState(newState); });
  }

 protected:
  void Host(
      winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view,
      TControl const& control) {
    m_control = control;
    ApplyControlChrome(m_control);
    auto weakThis = static_cast<TSelf*>(this)->get_weak();
    m_sizeRevoker = m_control.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        GuardedCall(L"SizeChanged", [&] { self->m_session.Invalidate(); });
      }
    });
    AttachEvents();
    m_session.Attach(view, m_control);
  }

  virtual void AttachEvents() {}
  virtual void DetachEvents() {}

  template <typename Fn>
  void WithEventsSuspended(Fn&& action) {
    DetachEvents();
    try {
      action();
    } catch (...) {
      AttachEvents();
      throw;
    }
    AttachEvents();
  }

  // Puts the control back on the last props after a user event so a parent
  // that ignores the event does not leave the native control drifted.
  template <typename Fn>
  void RestoreProps(Fn&& apply) {
    auto const& props = this->Props();
    if (!props || !m_control) {
      return;
    }
    WithEventsSuspended([&] { apply(*props); });
  }

  TControl m_control{nullptr};
  XamlIslandSession m_session;

 private:
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

template <typename TSelf>
void RegisterHostedControl(
    winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder,
    winrt::Windows::Foundation::Size fallback) {
  ConfigureXamlIsland(
      builder, fallback, [](winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
        GuardedCall(L"Attach", [&] {
          auto userData = winrt::make_self<TSelf>();
          // The generated UpdateProps handler dereferences UserData. Register it
          // before Attach, which can re-enter layout while the island connects.
          view.UserData(*userData);
          userData->Attach(view);
        });
      });
}

} // namespace winrt::Winui
