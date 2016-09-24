#include "stdafx.h"

#include "VST.Audio.h"

namespace VST {
	namespace Audio {
		DeviceStateChangedEventArgs::DeviceStateChangedEventArgs(DeviceState state) {
			this->_state = state;
		}

		DeviceState DeviceStateChangedEventArgs::State::get() {
			return this->_state;
		}
	}
}
