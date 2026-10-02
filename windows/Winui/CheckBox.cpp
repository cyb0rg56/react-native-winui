#include "pch.h"

#include "CheckBox.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/CheckBox.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct CheckBoxComponentView
    : winrt::implements<CheckBoxComponentView, winrt::IInspectable>,
      XamlComponentView<
          CheckBoxComponentView,
          winuiCodegen::BaseCheckBox<CheckBoxComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::CheckBox> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::CheckBox());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::CheckBoxProps> const& newProps,
      winrt::com_ptr<winuiCodegen::CheckBoxProps> const& oldProps) noexcept override {
    winuiCodegen::BaseCheckBox<CheckBoxComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"CheckBox.UpdateProps", [&] {
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
        if (!oldProps || oldProps->checked != newProps->checked || oldProps->indeterminate != newProps->indeterminate) {
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
        GuardedCall(L"CheckBox.Checked", [&] { self->EmitChange(); });
      }
    };
    m_checkedRevoker = m_control.Checked(winrt::auto_revoke, handler);
    m_uncheckedRevoker = m_control.Unchecked(winrt::auto_revoke, handler);
    m_indeterminateRevoker = m_control.Indeterminate(winrt::auto_revoke, handler);
  }

  void DetachEvents() override {
    m_checkedRevoker.revoke();
    m_uncheckedRevoker.revoke();
    m_indeterminateRevoker.revoke();
  }

 private:
  void ApplyChecked(winuiCodegen::CheckBoxProps const& props) {
    if (props.indeterminate.value_or(false)) {
      m_control.IsChecked(nullptr);
    } else {
      m_control.IsChecked(props.checked.value_or(false));
    }
  }

  void EmitChange() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::CheckBoxSpec_onCheckedChange args{};
    if (auto checked = m_control.IsChecked()) {
      args.checked = checked.Value();
      args.indeterminate = false;
    } else {
      args.checked = false;
      args.indeterminate = true;
    }
    emitter->onCheckedChange(std::move(args));
    RestoreProps([this](winuiCodegen::CheckBoxProps const& props) { ApplyChecked(props); });
  }

  winrt::Microsoft::UI::Xaml::Controls::CheckBox::Checked_revoker m_checkedRevoker;
  winrt::Microsoft::UI::Xaml::Controls::CheckBox::Unchecked_revoker m_uncheckedRevoker;
  winrt::Microsoft::UI::Xaml::Controls::CheckBox::Indeterminate_revoker m_indeterminateRevoker;
};

} // namespace winrt::Winui

void RegisterCheckBoxComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterCheckBoxNativeComponent<winrt::Winui::CheckBoxComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::CheckBoxComponentView>(builder, {160, 40});
      });
}
