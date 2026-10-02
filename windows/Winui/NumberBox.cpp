#include "pch.h"

#include "NumberBox.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/NumberBox.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct NumberBoxComponentView
    : winrt::implements<NumberBoxComponentView, winrt::IInspectable>,
      winuiCodegen::BaseNumberBox<NumberBoxComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_box = winrt::Microsoft::UI::Xaml::Controls::NumberBox();
    m_box.HorizontalAlignment(winrt::Microsoft::UI::Xaml::HorizontalAlignment::Stretch);
    auto weakThis = get_weak();
    m_sizeRevoker = m_box.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    AttachEvents();
    m_session.Attach(view, m_box);
  }

  ~NumberBoxComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::NumberBoxProps> const& newProps,
      winrt::com_ptr<winuiCodegen::NumberBoxProps> const&) noexcept override {
    BaseNumberBox::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_box) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_box, newProps->theme, newProps->disabled);
      m_box.PlaceholderText(ToHString(newProps->placeholder));
      if (newProps->minimum) {
        m_box.Minimum(*newProps->minimum);
      }
      if (newProps->maximum) {
        m_box.Maximum(*newProps->maximum);
      }
      if (newProps->step) {
        m_box.SmallChange(*newProps->step);
      }
      m_box.Value(newProps->value.value_or(0));
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
    m_valueRevoker = m_box.ValueChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->EmitValue();
      }
    });
  }

  void WithEventsSuspended(auto&& action) {
    m_valueRevoker.revoke();
    action();
    AttachEvents();
  }

  void EmitValue() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::NumberBoxSpec_onValueChange args{};
    args.value = m_box.Value();
    emitter->onValueChange(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::NumberBox m_box{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::NumberBox::ValueChanged_revoker m_valueRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterNumberBoxComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterNumberBoxNativeComponent<winrt::Winui::NumberBoxComponentView>(
      packageBuilder,
      [](winrt::Microsoft::ReactNative::Composition::IReactCompositionViewComponentBuilder const& builder) {
        winrt::Winui::ConfigureXamlIsland(
            builder, {160, 56}, [](auto const& view) {
              auto userData = winrt::make_self<winrt::Winui::NumberBoxComponentView>();
              userData->Attach(view);
              view.UserData(*userData);
            });
      });
}
