package app.organicmaps.media;

import android.content.Context;
import android.view.View;
import android.widget.TextView;
import androidx.annotation.NonNull;
import app.organicmaps.R;
import app.organicmaps.sdk.util.Config;
import app.organicmaps.util.UiUtils;

/// Shows the track played by another app on the map screen when not navigating.
/// During navigation the navigation panel shows it instead.
public class MapNowPlayingController
{
  @NonNull
  private final NowPlayingTracker mTracker;
  private boolean mStarted;
  private boolean mNavigating;

  public MapNowPlayingController(@NonNull Context context, @NonNull View root)
  {
    final TextView view = root.findViewById(R.id.free_drive_now_playing);
    mTracker = new NowPlayingTracker(context, text -> {
      view.setText(text);
      UiUtils.showIf(text != null, view);
    });
  }

  public void onStart()
  {
    mStarted = true;
    update();
  }

  public void onStop()
  {
    mStarted = false;
    update();
  }

  public void setNavigating(boolean navigating)
  {
    mNavigating = navigating;
    update();
  }

  private void update()
  {
    if (mStarted && !mNavigating && Config.isNowPlayingEnabled())
      mTracker.start();
    else
      mTracker.stop();
  }
}
