#include "pch.h"

#include "RatingControl.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/RatingControl.g.h"

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct RatingControlComponentView
    : winrt::implements<RatingControlComponentView, winrt::IInspectable>,
      XamlComponentView<
          RatingControlComponentView,
          winuiCodegen::BaseRatingControl<RatingControlComponentView>,
          winrt::Microsoft::UI::Xaml::Controls::RatingControl> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    Host(view, winrt::Microsoft::UI::Xaml::Controls::RatingControl());
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::RatingControlProps> const& newProps,
      winrt::com_ptr<winuiCodegen::RatingControlProps> const& oldProps) noexcept override {
    winuiCodegen::BaseRatingControl<RatingControlComponentView>::UpdateProps(view, newProps, oldProps);
    GuardedCall(L"RatingControl.UpdateProps", [&] {
      if (!newProps || !m_control) {
        return;
      }

      bool changed = !oldProps;
      WithEventsSuspended([&] {
        if (ChromeChanged(newProps, oldProps)) {
          PrepareElement(m_control, newProps->theme, newProps->disabled, newProps->ViewProps, m_session);
          changed = true;
        }
        if (!oldProps || oldProps->maxRating != newProps->maxRating || oldProps->value != newProps->value) {
          ApplyValue(*newProps);
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
    m_valueRevoker = m_control.ValueChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        GuardedCall(L"RatingControl.ValueChanged", [&] { self->EmitValue(); });
      }
    });
  }

  void DetachEvents() override {
    m_valueRevoker.revoke();
  }

 private:
  void ApplyValue(winuiCodegen::RatingControlProps const& props) {
    m_control.MaxRating(props.maxRating);
    m_control.Value(props.value.value_or(0));
  }

  void EmitValue() {
    auto const emitter = EventEmitter();
    if (!emitter) {
      return;
    }
    winuiCodegen::RatingControlSpec_onValueChange args{};
    args.value = m_control.Value();
    emitter->onValueChange(std::move(args));
    RestoreProps([this](winuiCodegen::RatingControlProps const& props) { ApplyValue(props); });
  }

  winrt::Microsoft::UI::Xaml::Controls::RatingControl::ValueChanged_revoker m_valueRevoker;
};

} // namespace winrt::Winui

void RegisterRatingControlComponentView(winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterRatingControlNativeComponent<winrt::Winui::RatingControlComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::RegisterHostedControl<winrt::Winui::RatingControlComponentView>(builder, {180, 40});
      });
}
