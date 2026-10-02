#include "pch.h"

#include "ReactPackageProvider.h"
#if __has_include("ReactPackageProvider.g.cpp")
#include "ReactPackageProvider.g.cpp"
#endif

#include "Winui.h"
#include "CheckBox.h"
#include "ComboBox.h"
#include "Expander.h"
#include "InfoBadge.h"
#include "InfoBar.h"
#include "NumberBox.h"
#include "ProgressBar.h"
#include "RadioButton.h"
#include "RatingControl.h"

using namespace winrt::Microsoft::ReactNative;

namespace winrt::Winui::implementation
{

void ReactPackageProvider::CreatePackage(IReactPackageBuilder const &packageBuilder) noexcept
{
  AddAttributedModules(packageBuilder, true);
  RegisterCheckBoxComponentView(packageBuilder);
  RegisterComboBoxComponentView(packageBuilder);
  RegisterExpanderComponentView(packageBuilder);
  RegisterInfoBadgeComponentView(packageBuilder);
  RegisterInfoBarComponentView(packageBuilder);
  RegisterNumberBoxComponentView(packageBuilder);
  RegisterProgressBarComponentView(packageBuilder);
  RegisterRadioButtonComponentView(packageBuilder);
  RegisterRatingControlComponentView(packageBuilder);
}

} // namespace winrt::Winui::implementation
