#include "pch.h"

#include "CheckBox.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/CheckBox.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct CheckBoxComponentView
    : winrt::implements<CheckBoxComponentView, winrt::IInspectable>,
      winuiCodegen::BaseCheckBox<CheckBoxComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_check = winrt::Microsoft::UI::Xaml::Controls::CheckBox();
    m_check.IsThreeState(true);
    auto weakThis = get_weak();
    m_sizeRevoker = m_check.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    AttachEvents();
    m_session.Attach(view, m_check);
  }

  ~CheckBoxComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::CheckBoxProps> const& newProps,
      winrt::com_ptr<winuiCodegen::CheckBoxProps> const&) noexcept override {
    BaseCheckBox::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_check) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_check, newProps->theme, newProps->disabled);
      m_check.Content(winrt::box_value(ToHString(newProps->label)));
      if (newProps->indeterminate.value_or(false)) {
        m_check.IsChecked(nullptr);
      } else {
        m_check.IsChecked(newProps->checked.value_or(false));
      }
    });
    m_session.Invalidate();
  }

  void UpdateState(
      winrt::Microsoft::ReactNative::ComponentView const&,
      winrt::Microsoft::ReactNative::IComponentState const& newState) noexcept override {
    m_state = newState;
    m_session.SetState(newState);
  }

 private:
  void AttachEvents() {
    auto weakThis = get_weak();
    auto handler = [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->EmitChange();
      }
    };
    m_checkedRevoker = m_check.Checked(winrt::auto_revoke, handler);
    m_uncheckedRevoker = m_check.Unchecked(winrt::auto_revoke, handler);
    m_indeterminateRevoker = m_check.Indeterminate(winrt::auto_revoke, handler);
  }

  void WithEventsSuspended(auto&& action) {
    m_checkedRevoker.revoke();
    m_uncheckedRevoker.revoke();
    m_indeterminateRevoker.revoke();
    action();
    AttachEvents();
  }

  void EmitChange() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::CheckBoxSpec_onCheckedChange args{};
    if (auto checked = m_check.IsChecked()) {
      args.checked = checked.Value();
      args.indeterminate = false;
    } else {
      args.checked = false;
      args.indeterminate = true;
    }
    emitter->onCheckedChange(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::CheckBox m_check{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::CheckBox::Checked_revoker m_checkedRevoker;
  winrt::Microsoft::UI::Xaml::Controls::CheckBox::Unchecked_revoker m_uncheckedRevoker;
  winrt::Microsoft::UI::Xaml::Controls::CheckBox::Indeterminate_revoker m_indeterminateRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterCheckBoxComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterCheckBoxNativeComponent<winrt::Winui::CheckBoxComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {280, 32}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::CheckBoxComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
