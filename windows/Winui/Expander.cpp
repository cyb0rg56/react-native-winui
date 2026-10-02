#include "pch.h"

#include "Expander.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/Expander.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

// Header and body are strings. React children are not hosted inside the island.
struct ExpanderComponentView
    : winrt::implements<ExpanderComponentView, winrt::IInspectable>,
      XamlComponentView<
          ExpanderComponentView,
          winuiCodegen::BaseExpander<ExpanderComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::Expander> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    auto expander = winrt::Microsoft::UI::Xaml::Controls::Expander();
    m_content = winrt::Microsoft::UI::Xaml::Controls::TextBlock();
    m_content.TextWrapping(winrt::Microsoft::UI::Xaml::TextWrapping::Wrap);
    expander.Content(m_content);
    Host(view, expander);
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::ExpanderProps> const& newProps,
      winrt::com_ptr<winuiCodegen::ExpanderProps> const& oldProps) noexcept override {
    winuiCodegen::BaseExpander<ExpanderComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"Expander.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      WithEventsSuspended([&] {
        if (ChromeChanged(newProps, oldProps)) {
          PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
          changed = true;
        }
        if (!oldProps || oldProps->header != newProps->header) {
          m_control.Header(winrt::box_value(ToHString(newProps->header)));
          changed = true;
        }
        if (!oldProps || oldProps->content != newProps->content) {
          m_content.Text(ToHString(newProps->content));
          changed = true;
        }
        if (!oldProps || oldProps->isExpanded != newProps->isExpanded) {
          ApplyExpanded(*newProps);
          changed = true;
        }
      });
      if (changed) {
        m_session.Invalidate();
      }
    });
  }

 protected:
  void AttachEvents() override {
    auto weakThis = get_weak();
    auto handler = [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        GuardedCall(L"Expander.Expanding", [&] { self->EmitChange(); });
      }
    };
    m_expandingRevoker = m_control.Expanding(winrt::auto_revoke, handler);
    m_collapsedRevoker = m_control.Collapsed(winrt::auto_revoke, handler);
  }

  void DetachEvents() override {
    m_expandingRevoker.revoke();
    m_collapsedRevoker.revoke();
  }

 private:
  void ApplyExpanded(winuiCodegen::ExpanderProps const& props) {
    m_control.IsExpanded(props.isExpanded.value_or(false));
  }

  void EmitChange() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::ExpanderSpec_onExpandChange args{};
    args.isExpanded = m_control.IsExpanded();
    emitter->onExpandChange(std::move(args));
    RestoreProps([this](winuiCodegen::ExpanderProps const& props) { ApplyExpanded(props); });
  }

  winrt::Microsoft::UI::Xaml::Controls::TextBlock m_content{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::Expander::Expanding_revoker m_expandingRevoker;
  winrt::Microsoft::UI::Xaml::Controls::Expander::Collapsed_revoker m_collapsedRevoker;
};

} // namespace winrt::Winui

void RegisterExpanderComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterExpanderNativeComponent<winrt::Winui::ExpanderComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::ExpanderComponentView>(builder, {320, 56});
      });
}
