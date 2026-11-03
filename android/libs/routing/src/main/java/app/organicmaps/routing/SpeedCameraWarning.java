package app.organicmaps.routing;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import app.organicmaps.sdk.util.Config;

// Speed camera alert sound: the beep or a user-defined phrase.
public final class SpeedCameraWarning
{
  private SpeedCameraWarning() {}

  @NonNull
  public static String getPhraseText(@NonNull Context context)
  {
    final String text = Config.TTS.getSpeedCamPhraseText().trim();
    return text.isEmpty() ? context.getString(R.string.speed_camera_alert_default_phrase) : text;
  }

  // Returns null when the beep should be played, also when voice is off to never stay silent.
  @Nullable
  public static String getPhrase(@NonNull Context context)
  {
    if (!Config.TTS.isSpeedCamPhrase() || !Config.TTS.isEnabled())
      return null;
    return getPhraseText(context);
  }
}
