// VST.Audio.h

#pragma once

using namespace System;

namespace VST {
	namespace Audio {
		ref class Controller;

		/// <summary>
		/// Represents an audio endpoint
		/// </summary>
		public ref class Endpoint {
		private:
			Controller^ Utils;
			String^ _id;
		internal:
			Endpoint(Controller^ utils, String^ Id);
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
			/// Gets the friendly name of the endpoint
			/// </summary>
			property String^ Name {
				String^ get();
			}

			/// <summary>
			/// Gets whether the endpoint is the current default device
			/// </summary>
			property bool IsDefault {
				bool get();
			};

			/// <summary>
			/// Gets the current volume of the endpoint
			/// </summary>
			/// <returns>The volume</returns>
			float GetVolume();

			/// <summary>
			/// Sets the volume of the endpoint
			/// </summary>
			/// <param name="volume">The volume</param>
			void SetVolume(float volume);

			/// <summary>
			/// Gets the current mute status of the endpoint
			/// </summary>
			/// <returns>The mute status</returns>
			bool GetMute();

			/// <summary>
			/// Sets the mute status of the endpoint
			/// </summary>
			/// <param name="mute">The mute status</param>
			void SetMute(bool mute);

			virtual bool Equals(Object^ otherEndpoint) override;
			virtual bool Equals(Endpoint^ otherEndpoint);
			virtual int GetHashCode() override;
			static bool operator!= (Endpoint^, Endpoint^);
			static bool operator== (Endpoint^, Endpoint^);
			virtual String^ ToString() override;
		};

		public ref class Controller {
		public:
			Controller();
			~Controller();

			/// <summary>
			/// Gets all the render audio endpoints currently enabled on the system
			/// </summary>
			/// <returns>The endpoints</returns>
			array<Endpoint^>^ GetAudioEndpoints();

			/// <summary>
			/// Gets the current default render endpoint
			/// </summary>
			/// <returns>The endpoint</returns>
			Endpoint^ GetDefaultAudioEndpoint();

			/// <summary>
			/// Sets the default render endpoint
			/// </summary>
			/// <param name="endpoint">The endpoint</param>
			void SetDefaultAudioEndpoint(Endpoint^ endpoint);
		};
	}
}
