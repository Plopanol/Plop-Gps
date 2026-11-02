package app.organicmaps.media;

import android.service.notification.NotificationListenerService;

/// Holds the notification access needed by MediaSessionManager.getActiveSessions(): notifications are never read.
/// Never toggle it with PackageManager: once re-enabled, the system does not restore the access until it is regranted.
public class NowPlayingListenerService extends NotificationListenerService
{}
