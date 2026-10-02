#pragma once

#include "pch.h"
#include "resource.h"

#include "NativeModules.h"

namespace winrt::Winui
{

// Package entry kept so the cpp-lib project still has an attributed module.
// Controls are registered as Fabric view components from ReactPackageProvider.
REACT_MODULE(Winui)
struct Winui
{
  REACT_INIT(Initialize)
  void Initialize(React::ReactContext const& reactContext) noexcept;

private:
  React::ReactContext m_context{nullptr};
};

} // namespace winrt::Winui
