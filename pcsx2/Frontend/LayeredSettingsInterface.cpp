/*  AetherSX2 - PS2 Emulator for Android and ARM PCs
 *  Copyright (C) 2022 AetherSX2 Dev Team
 *
 *  AetherSX2 is provided under the terms of the Creative Commons
 *  Attribution-NonCommercial-NoDerivatives International License
 *  (BY-NC-ND 4.0, https://creativecommons.org/licenses/by-nc-nd/4.0/).
 * 
 *  Commercialization of this application and source code is forbidden.
 */

#include "PrecompiledHeader.h"

#include "LayeredSettingsInterface.h"
#include "common/Assertions.h"

LayeredSettingsInterface::LayeredSettingsInterface() = default;

LayeredSettingsInterface::~LayeredSettingsInterface() = default;

bool LayeredSettingsInterface::Save()
{
	pxFailRel("Attempting to save layered settings interface");
	return false;
}

void LayeredSettingsInterface::Clear()
{
	pxFailRel("Attempting to clear layered settings interface");
}

bool LayeredSettingsInterface::GetIntValue(const char* section, const char* key, int* value) const
{
	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			if (sif->GetIntValue(section, key, value))
				return true;
		}
	}

	return false;
}

bool LayeredSettingsInterface::GetUIntValue(const char* section, const char* key, uint* value) const
{
	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			if (sif->GetUIntValue(section, key, value))
				return true;
		}
	}

	return false;
}

bool LayeredSettingsInterface::GetFloatValue(const char* section, const char* key, float* value) const
{
	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			if (sif->GetFloatValue(section, key, value))
				return true;
		}
	}

	return false;
}

bool LayeredSettingsInterface::GetDoubleValue(const char* section, const char* key, double* value) const
{
	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			if (sif->GetDoubleValue(section, key, value))
				return true;
		}
	}

	return false;
}

bool LayeredSettingsInterface::GetBoolValue(const char* section, const char* key, bool* value) const
{
	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			if (sif->GetBoolValue(section, key, value))
				return true;
		}
	}

	return false;
}

bool LayeredSettingsInterface::GetStringValue(const char* section, const char* key, std::string* value) const
{
	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			if (sif->GetStringValue(section, key, value))
				return true;
		}
	}

	return false;
}

void LayeredSettingsInterface::SetIntValue(const char* section, const char* key, int value)
{
	pxFailRel("Attempt to call SetIntValue() on layered settings interface");
}

void LayeredSettingsInterface::SetUIntValue(const char* section, const char* key, uint value)
{
	pxFailRel("Attempt to call SetUIntValue() on layered settings interface");
}

void LayeredSettingsInterface::SetFloatValue(const char* section, const char* key, float value)
{
	pxFailRel("Attempt to call SetFloatValue() on layered settings interface");
}

void LayeredSettingsInterface::SetDoubleValue(const char* section, const char* key, double value)
{
	pxFailRel("Attempt to call SetDoubleValue() on layered settings interface");
}

void LayeredSettingsInterface::SetBoolValue(const char* section, const char* key, bool value)
{
	pxFailRel("Attempt to call SetBoolValue() on layered settings interface"); 
}

void LayeredSettingsInterface::SetStringValue(const char* section, const char* key, const char* value)
{
	pxFailRel("Attempt to call SetStringValue() on layered settings interface");
}

void LayeredSettingsInterface::DeleteValue(const char* section, const char* key)
{
	pxFailRel("Attempt to call DeleteValue() on layered settings interface");
}

void LayeredSettingsInterface::ClearSection(const char* section)
{
	pxFailRel("Attempt to call ClearSection() on layered settings interface");
}

std::vector<std::string> LayeredSettingsInterface::GetStringList(const char* section, const char* key)
{
	std::vector<std::string> ret;

	for (u32 layer = FIRST_LAYER; layer <= LAST_LAYER; layer++)
	{
		if (SettingsInterface* sif = m_layers[layer]; sif != nullptr)
		{
			ret = sif->GetStringList(section, key);
			if (!ret.empty())
				break;
		}
	}

	return ret;
}

void LayeredSettingsInterface::SetStringList(const char* section, const char* key, const std::vector<std::string>& items)
{
	pxFailRel("Attempt to call SetStringList() on layered settings interface");
}

bool LayeredSettingsInterface::RemoveFromStringList(const char* section, const char* key, const char* item)
{
	pxFailRel("Attempt to call RemoveFromStringList() on layered settings interface");
	return false;
}

bool LayeredSettingsInterface::AddToStringList(const char* section, const char* key, const char* item)
{
	pxFailRel("Attempt to call AddToStringList() on layered settings interface");
	return true;
}
