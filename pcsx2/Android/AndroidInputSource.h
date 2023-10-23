#pragma once
#include "Frontend/InputSource.h"

class AndroidInputSource final : public InputSource
{
public:
  AndroidInputSource();
  ~AndroidInputSource();
  bool Initialize(SettingsInterface& si) override;
  void UpdateSettings(SettingsInterface& si) override;
  void Shutdown() override;
  void PollEvents() override;
  std::vector<std::pair<std::string, std::string>> EnumerateDevices() override;
  std::vector<InputBindingKey> EnumerateMotors() override;
  bool GetGenericBindingMapping(const std::string_view& device, GenericInputBindingMapping* mapping) override;
  void UpdateMotorState(InputBindingKey key, float intensity) override;
  std::optional<InputBindingKey> ParseKeyString(const std::string_view& device, const std::string_view& binding) override;
  std::string ConvertKeyToString(InputBindingKey key) override;
};
