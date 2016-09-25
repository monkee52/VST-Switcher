// VST.Audio.h

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace VST {
	namespace Audio {
		[Flags]
		public enum class DeviceState : DWORD {
			Active = DEVICE_STATE_ACTIVE,
			Disabled = DEVICE_STATE_DISABLED,
			NotPresent = DEVICE_STATE_NOTPRESENT,
			Unplugged = DEVICE_STATE_UNPLUGGED
		};

		public enum class DeviceType {
			Render = eRender,
			Capture = eCapture,
			All = eAll
		};

		public enum class DeviceRole {
			Console = eConsole,
			Multimedia = eMultimedia,
			Communications = eCommunications
		};

		public ref class DeviceStateChangedEventArgs : EventArgs {
		private:
			DeviceState _state;
		public:
			DeviceStateChangedEventArgs(DeviceState state);
			
			property DeviceState State {
				DeviceState get();
			}
		};

		public ref class DefaultDeviceChangedEventArgs : EventArgs {
		private:
			DeviceType _type;
			DeviceRole _role;
		public:
			DefaultDeviceChangedEventArgs(DeviceType type, DeviceRole role);

			property DeviceType Type {
				DeviceType get();
			}

			property DeviceRole Role {
				DeviceRole get();
			}
		};

		ref class Controller;

		private class CMMNotificationClient : public IMMNotificationClient {
		private:
			LONG _cRef;
			GCHandle hController;
		public:
			CMMNotificationClient(void* pController);
			~CMMNotificationClient();

			ULONG STDMETHODCALLTYPE AddRef();
			ULONG STDMETHODCALLTYPE Release();
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface);

			HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDevice);
			HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
			HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
			HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
			HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
		};

		/// <summary>
		/// Represents an audio endpoint
		/// </summary>
		public ref class Device {
		private:
			Controller^ Utils;
			String^ _id;
		internal:
			Device(Controller^ utils, String^ Id);
		public:
			/// <summary>
			/// Gets the system defined identifier for the endpoint
			/// </summary>
			property String^ Id {
				String^ get();

			private:
				void set(String^ id);
			};

			/// <summary>
			/// Gets the name of the endpoint. e.g. "XYZ Audio Adapter"
			/// </summary>
			property String^ Name {
				String^ get();
			}

			/// <summary>
			/// Gets the description of the endpoint. e.g. "Speakers (XYZ Audio Adapter)"
			/// </summary>
			property String^ FriendlyName {
				String^ get();
			}

			/// <summary>
			/// Gets the description of the endpoint. e.g. "Speakers"
			/// </summary>
			property String^ Description {
				String^ get();
			}

			/// <summary>
			/// Gets whether the endpoint is the current default device
			/// </summary>
			/// <param name="type">The type of the endpoint</param>
			/// <param name="role">The role of the endpoint</param>
			/// <returns>Whether the endpoint is the default for the type and role</returns>
			bool IsDefault(DeviceType type, DeviceRole role);

			/// <summary>
			/// Gets the current state of the endpoint
			/// </summary>
			property DeviceState State {
				DeviceState get();
			}

			/// <summary>
			/// Gets or sets the volume of the endpoint
			/// </summary>
			property float Volume {
				float get();
				void set(float volume);
			}

			/// <summary>
			/// Gets or sets the mute status of the endpoint
			/// </summary>
			property bool Muted {
				bool get();
				void set(bool mute);
			}

			virtual bool Equals(Object^ otherDevice) override;
			virtual bool Equals(Device^ otherDevice);
			virtual int GetHashCode() override;
			static bool operator!= (Device^, Device^);
			static bool operator== (Device^, Device^);
			virtual String^ ToString() override;
		};

		public ref class Controller : public IDisposable {
		private:
			CMMNotificationClient* notificationClient;
		internal:
			void FireDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDevice);
			void FireDeviceAdded(LPCWSTR pwstrDeviceId);
			void FireDeviceRemoved(LPCWSTR pwstrDeviceId);
			void FireDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
			void FirePropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
		public:
			Controller();
			~Controller();
			!Controller();

			event EventHandler<DefaultDeviceChangedEventArgs^>^ OnDefaultDeviceChanged;
			event EventHandler^ OnDeviceAdded;
			event EventHandler^ OnDeviceRemoved;
			event EventHandler<DeviceStateChangedEventArgs^>^ OnDeviceStateChanged;
			//event EventHandler^ OnPropertyValueChanged;

			/// <summary>
			/// Gets all the render audio endpoints currently enabled on the system
			/// </summary>
			/// <returns>The endpoints</returns>
			array<Device^>^ GetAudioDevices(DeviceType dataFlow, DeviceState stateMask);

			/// <summary>
			/// Gets an audio endpoint by id
			/// </summary>
			/// <param name="id">The endpoint id</param>
			/// <returns>The endpoint</returns>
			Device^ GetAudioDevice(String^ id);

			/// <summary>
			/// Gets the current default render endpoint
			/// </summary>
			/// <returns>The endpoint</returns>
			Device^ GetDefaultAudioDevice(DeviceType dataFlow, DeviceRole role);

			/// <summary>
			/// Sets the default render endpoint
			/// </summary>
			/// <param name="endpoint">The endpoint</param>
			void SetDefaultAudioDevice(Device^ endpoint, DeviceRole role);
		};
	}
}
