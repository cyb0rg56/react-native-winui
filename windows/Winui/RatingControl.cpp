#include "pch.h"

#include "RatingControl.h"

#include "XamlControl.h"
#include "codegen/react/components/RNWinuiSpec/RatingControl.g.h"

#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace winrt::Winui {

struct RatingControlComponentView
    : winrt::implements<RatingControlComponentView, winrt::IInspectable>,
      winuiCodegen::BaseRatingControl<RatingControlComponentView> {
  void Attach(winrt::Microsoft::ReactNative::Composition::ContentIslandComponentView const& view) {
    m_rating = winrt::Microsoft::UI::Xaml::Controls::RatingControl();
    winrt::Microsoft::UI::Xaml::Automation::AutomationProperties::SetName(m_rating, L"Rating");
    auto weakThis = get_weak();
    m_sizeRevoker = m_rating.SizeChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
      if (auto self = weakThis.get()) {
        self->m_session.Invalidate();
      }
    });
    AttachEvents();
    m_session.Attach(view, m_rating);
  }

  ~RatingControlComponentView() {
    m_session.Close();
  }

  void UpdateProps(
      winrt::Microsoft::ReactNative::ComponentView const& view,
      winrt::com_ptr<winuiCodegen::RatingControlProps> const& newProps,
      winrt::com_ptr<winuiCodegen::RatingControlProps> const&) noexcept override {
    BaseRatingControl::UpdateProps(view, newProps, nullptr);
    if (!newProps || !m_rating) {
      return;
    }

    WithEventsSuspended([&] {
      PrepareElement(m_rating, newProps->theme, newProps->disabled);
      m_rating.MaxRating(newProps->maxRating);
      m_rating.Value(newProps->value.value_or(0));
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
    m_valueRevoker = m_rating.ValueChanged(winrt::auto_revoke, [weakThis](auto const&, auto const&) {
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
    winuiCodegen::RatingControlSpec_onValueChange args{};
    args.value = m_rating.Value();
    emitter->onValueChange(std::move(args));
  }

  XamlIslandSession m_session;
  winrt::Microsoft::UI::Xaml::Controls::RatingControl m_rating{nullptr};
  winrt::Microsoft::ReactNative::IComponentState m_state{nullptr};
  winrt::Microsoft::UI::Xaml::Controls::RatingControl::ValueChanged_revoker m_valueRevoker;
  winrt::Microsoft::UI::Xaml::FrameworkElement::SizeChanged_revoker m_sizeRevoker;
};

} // namespace winrt::Winui

void RegisterRatingControlComponentView(
    winrt::Microsoft::ReactNative::IReactPackageBuilder const& packageBuilder) {
  winuiCodegen::RegisterRatingControlNativeComponent<winrt::Winui::RatingControlComponentView>(
      packageBuilder, [](auto const& builder) {
        winrt::Winui::ConfigureXamlIsland(builder, {220, 40}, [](auto const& view) {
          auto userData = winrt::make_self<winrt::Winui::RatingControlComponentView>();
          userData->Attach(view);
          view.UserData(*userData);
        });
      });
}
