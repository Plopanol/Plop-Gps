package app.organicmaps.routing;

import android.content.Context;
import androidx.annotation.MainThread;
import androidx.annotation.NonNull;
import app.organicmaps.sdk.Framework;
import app.organicmaps.sdk.routing.SpeedCameraZoneListener;
import app.organicmaps.sdk.sound.MediaPlayerWrapper;
import app.organicmaps.sdk.sound.TtsPlayer;

// Beep and voice alert for fixed speed camera danger zones, also outside of navigation.
public final class SpeedCameraZoneAlert implements SpeedCameraZoneListener
{
  @NonNull
  private final Context mContext;
  @NonNull
  private final MediaPlayerWrapper mPlayer;

  private SpeedCameraZoneAlert(@NonNull Context context)
  {
    mContext = context;
    mPlayer = new MediaPlayerWrapper(context);
  }

  @MainThread
  public static void initialize(@NonNull Context context)
  {
    Framework.nativeSetSpeedCameraZoneListener(new SpeedCameraZoneAlert(context.getApplicationContext()));
  }

  @Override
  public void onSpeedCameraZoneEvent(boolean entered, int maxSpeedKmph)
  {
    if (!entered)
    {
      TtsPlayer.INSTANCE.speak(mContext.getString(R.string.speed_camera_zone_left));
      return;
    }

    mPlayer.playback(R.raw.speed_cams_beep);
    TtsPlayer.INSTANCE.speak(maxSpeedKmph > 0
                                 ? mContext.getString(R.string.speed_camera_zone_entered, maxSpeedKmph)
                                 : mContext.getString(R.string.speed_camera_zone_entered_no_limit));
  }
}
