package app.organicmaps.widget;

import android.location.Location;
import android.os.SystemClock;
import android.util.Pair;
import android.view.View;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import app.organicmaps.R;
import app.organicmaps.sdk.Framework;
import app.organicmaps.sdk.location.LocationHelper;
import app.organicmaps.sdk.util.StringUtils;
import app.organicmaps.sdk.widgets.speedlimit.SpeedLimitView;
import app.organicmaps.util.ThemeUtils;
import app.organicmaps.util.UiUtils;

/// Shows the current speed and the road speed limit on the map screen when driving without navigation.
public class FreeDriveSpeedController
{
  // Keeps the widget visible during short stops (traffic lights, junctions).
  private static final long HIDE_DELAY_MS = 10_000;

  @NonNull
  private final View mFrame;
  @NonNull
  private final View mSpeedContainer;
  @NonNull
  private final TextView mSpeedValue;
  @NonNull
  private final TextView mSpeedUnits;
  @NonNull
  private final SpeedLimitView mSpeedLimit;

  private long mLastDrivingTimeMs = 0;

  public FreeDriveSpeedController(@NonNull View root)
  {
    mFrame = root.findViewById(R.id.free_drive_speed);
    mSpeedContainer = mFrame.findViewById(R.id.free_drive_speed_container);
    mSpeedValue = mFrame.findViewById(R.id.free_drive_speed_value);
    mSpeedUnits = mFrame.findViewById(R.id.free_drive_speed_units);
    mSpeedLimit = mFrame.findViewById(R.id.free_drive_speed_limit);
  }

  public void update(@NonNull Location location)
  {
    final long now = SystemClock.elapsedRealtime();
    final float speed = location.hasSpeed() ? location.getSpeed() : 0;
    if (speed >= LocationHelper.FREE_DRIVE_MIN_SPEED_MPS)
      mLastDrivingTimeMs = now;
    else if (mLastDrivingTimeMs == 0 || now - mLastDrivingTimeMs > HIDE_DELAY_MS)
    {
      hide();
      return;
    }

    UiUtils.show(mFrame);

    final Pair<String, String> speedAndUnits = StringUtils.nativeFormatSpeedAndUnits(speed);
    mSpeedValue.setText(speedAndUnits.first);
    mSpeedUnits.setText(speedAndUnits.second);

    final double speedLimitMps = Framework.nativeGetFreeDriveSpeedLimitMps();
    final boolean exceeded = speedLimitMps > 0 && speed > speedLimitMps;
    final int textColor = exceeded ? ContextCompat.getColor(mFrame.getContext(), R.color.white_primary)
                                   : ThemeUtils.getColor(mFrame.getContext(), android.R.attr.textColorPrimary);
    mSpeedValue.setTextColor(textColor);
    mSpeedUnits.setTextColor(textColor);
    mSpeedContainer.setActivated(exceeded);

    if (speedLimitMps > 0)
    {
      mSpeedLimit.setSpeedLimit(StringUtils.nativeFormatSpeed(speedLimitMps), exceeded);
      UiUtils.show(mSpeedLimit);
    }
    else
      UiUtils.hide(mSpeedLimit);
  }

  public void hide()
  {
    mLastDrivingTimeMs = 0;
    UiUtils.hide(mFrame);
  }
}
