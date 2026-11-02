package app.organicmaps.media;

import android.content.ComponentName;
import android.content.Context;
import android.media.MediaMetadata;
import android.media.session.MediaController;
import android.media.session.MediaSessionManager;
import android.media.session.PlaybackState;
import android.text.TextUtils;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.NotificationManagerCompat;
import app.organicmaps.sdk.util.log.Logger;
import java.util.ArrayList;
import java.util.List;

/// Reports the track currently played by any app, using the system media sessions.
public class NowPlayingTracker implements MediaSessionManager.OnActiveSessionsChangedListener
{
  private static final String TAG = NowPlayingTracker.class.getSimpleName();

  public interface Listener
  {
    /// @param text "Title — Artist", or null when nothing is playing.
    void onNowPlayingChanged(@Nullable String text);
  }

  @NonNull
  private final MediaSessionManager mManager;
  @NonNull
  private final ComponentName mComponent;
  @NonNull
  private final Listener mListener;
  @NonNull
  private final List<MediaController> mControllers = new ArrayList<>();
  private boolean mStarted;

  private final MediaController.Callback mCallback = new MediaController.Callback() {
    @Override
    public void onPlaybackStateChanged(@Nullable PlaybackState state)
    {
      notifyListener();
    }

    @Override
    public void onMetadataChanged(@Nullable MediaMetadata metadata)
    {
      notifyListener();
    }

    @Override
    public void onSessionDestroyed()
    {
      notifyListener();
    }
  };

  public NowPlayingTracker(@NonNull Context context, @NonNull Listener listener)
  {
    mManager = (MediaSessionManager) context.getSystemService(Context.MEDIA_SESSION_SERVICE);
    mComponent = new ComponentName(context, NowPlayingListenerService.class);
    mListener = listener;
  }

  public static boolean isAccessGranted(@NonNull Context context)
  {
    return NotificationManagerCompat.getEnabledListenerPackages(context).contains(context.getPackageName());
  }

  public void start()
  {
    if (mStarted)
      return;
    try
    {
      mManager.addOnActiveSessionsChangedListener(this, mComponent);
      mStarted = true;
      onActiveSessionsChanged(mManager.getActiveSessions(mComponent));
    }
    catch (SecurityException e)
    {
      // Notification access was not granted or was revoked.
      Logger.w(TAG, "No access to media sessions", e);
      stop();
    }
  }

  public void stop()
  {
    if (mStarted)
      mManager.removeOnActiveSessionsChangedListener(this);
    mStarted = false;
    clearControllers();
    mListener.onNowPlayingChanged(null);
  }

  @Override
  public void onActiveSessionsChanged(@Nullable List<MediaController> controllers)
  {
    clearControllers();
    if (controllers != null)
    {
      for (MediaController controller : controllers)
      {
        controller.registerCallback(mCallback);
        mControllers.add(controller);
      }
    }
    notifyListener();
  }

  private void clearControllers()
  {
    for (MediaController controller : mControllers)
      controller.unregisterCallback(mCallback);
    mControllers.clear();
  }

  private void notifyListener()
  {
    mListener.onNowPlayingChanged(getPlayingText());
  }

  @Nullable
  private String getPlayingText()
  {
    // Sessions are sorted by priority, so the first playing one is what the user hears.
    for (MediaController controller : mControllers)
    {
      final PlaybackState state = controller.getPlaybackState();
      if (state == null || state.getState() != PlaybackState.STATE_PLAYING)
        continue;
      final MediaMetadata metadata = controller.getMetadata();
      if (metadata == null)
        continue;
      final String title = metadata.getString(MediaMetadata.METADATA_KEY_TITLE);
      if (TextUtils.isEmpty(title))
        continue;
      final String artist = metadata.getString(MediaMetadata.METADATA_KEY_ARTIST);
      return TextUtils.isEmpty(artist) ? title : title + " — " + artist;
    }
    return null;
  }
}
