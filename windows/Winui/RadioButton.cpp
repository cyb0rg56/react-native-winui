#include "pch.h"

#include "RadioButton.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/RadioButton.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct RadioButtonComponentView
    : winrt::implements<RadioButtonComponentView, winrt::IInspectable>,
      XamlComponentView<
          RadioButtonComponentView,
          winuiCodegen::BaseRadioButton<RadioButtonComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::RadioButton> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::RadioButton());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::RadioButtonProps> const& newProps,
      winrt::com_ptr<winuiCodegen::RadioButtonProps> const& oldProps) noexcept override {
    winuiCodegen::BaseRadioButton<RadioButtonComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"RadioButton.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      WithEventsSuspended([&] {
        if (ChromeChanged(newProps, oldProps)) {
          PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
          changed = true;
        }
        if (!oldProps || oldProps->label != newProps->label) {
          m_control.Content(winrt::box_value(ToHString(newProps->label)));
          changed = true;
        }
        // GroupName is scoped to one XAML island, so it does not uncheck
        // radios hosted by sibling component views. RadioGroup owns that.
        if (!oldProps || oldProps->group != newProps->group) {
          m_control.GroupName(ToHString(newProps->group));
          changed = true;
        }
        if (!oldProps || oldProps->checked != newProps->checked) {
          ApplyChecked(*newProps);
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
        GuardedCall(L"RadioButton.Checked", [&] { self->EmitChange(); });
      }
    };
    m_checkedRevoker = m_control.Checked(winrt::auto_revoke, handler);
    m_uncheckedRevoker = m_control.Unchecked(winrt::auto_revoke, handler);
  }

  void DetachEvents() override {
    m_checkedRevoker.revoke();
    m_uncheckedRevoker.revoke();
  }

 private:
  void ApplyChecked(winuiCodegen::RadioButtonProps const& props) {
    m_control.IsChecked(props.checked.value_or(false));
  }

  void EmitChange() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::RadioButtonSpec_onCheckedChange args{};
    if (auto checked = m_control.IsChecked()) {
      args.checked = checked.Value();
    }
    emitter->onCheckedChange(std::move(args));
    RestoreProps([this](winuiCodegen::RadioButtonProps const& props) { ApplyChecked(props); });
  }

  winrt::Microsoft::UI::Xaml::Controls::RadioButton::Checked_revoker m_checkedRevoker;
  winrt::Microsoft::UI::Xaml::Controls::RadioButton::Unchecked_revoker m_uncheckedRevoker;
};

} // namespace winrt::Winui

void RegisterRadioButtonComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterRadioButtonNativeComponent<winrt::Winui::RadioButtonComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::RadioButtonComponentView>(builder, {160, 40});
      });
}
