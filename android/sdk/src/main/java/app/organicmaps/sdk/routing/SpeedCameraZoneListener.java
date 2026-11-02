package app.organicmaps.sdk.routing;

import androidx.annotation.Keep;
import androidx.annotation.MainThread;

public interface SpeedCameraZoneListener
{
  // Called from JNI on entering (|entered| is true) or leaving a speed camera danger zone.
  // |maxSpeedKmph| is 0 if unknown or on leaving.
  @Keep
  @SuppressWarnings("unused")
  @MainThread
  void onSpeedCameraZoneEvent(boolean entered, int maxSpeedKmph);
}
