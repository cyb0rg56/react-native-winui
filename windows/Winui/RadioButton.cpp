#include "pch.h"

#include "RadioButton.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/RadioButton.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct RadioButtonComponentView
    : winrt::implements<RadioButtonComponentView, winrt::IInspectable>,
      winuiCodegen::BaseRadioButton<RadioButtonComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_radio = winrt::Microsoft::UI::Xaml::Controls::RadioButton();
    auto weakThis = get_weak();
    m_sizeRevoker = m_radio.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    AttachEvents();
    m_session.Attach(view, m_radio);
  }

  ~RadioButtonComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::RadioButtonProps> const& newProps,
      winrt::com_ptr<winuiCodegen::RadioButtonProps> const&) noexcept override {
    BaseRadioButton::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_radio) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_radio, newProps->theme, newProps->disabled);
      m_radio.Content(winrt::box_value(ToHString(newProps->label)));
      m_radio.GroupName(ToHString(newProps->group));
      m_radio.IsChecked(newProps->checked.value_or(false));
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
    m_checkedRevoker = m_radio.Checked(winrt::auto_revoke, handler);
    m_uncheckedRevoker = m_radio.Unchecked(winrt::auto_revoke, handler);
  }

  void WithEventsSuspended(auto&& action) {
    m_checkedRevoker.revoke();
    m_uncheckedRevoker.revoke();
    action();
    AttachEvents();
  }

  void EmitChange() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::RadioButtonSpec_onCheckedChange args{};
    if (auto checked = m_radio.IsChecked()) {
      args.checked = checked.Value();
    }
    emitter->onCheckedChange(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::RadioButton m_radio{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::RadioButton::Checked_revoker m_checkedRevoker;
  winrt::Microsoft::UI::Xaml::Controls::RadioButton::Unchecked_revoker m_uncheckedRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterRadioButtonComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterRadioButtonNativeComponent<winrt::Winui::RadioButtonComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {200, 32}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::RadioButtonComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
