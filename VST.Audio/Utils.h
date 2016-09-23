#pragma once

using namespace System;

String^ ConvertHrToString(HRESULT hr);
void VolumeCommon(String^ id, bool set, float* volume, bool* mute);
String^ GetPropertyCommon(String^ id, const PROPERTYKEY key);
