package app.organicmaps.media;

import android.service.notification.NotificationListenerService;

/// Holds the notification access needed by MediaSessionManager.getActiveSessions(): notifications are never read.
/// Enabled only while the "now playing" preference is on, see NowPlayingTracker.setListenerEnabled().
public class NowPlayingListenerService extends NotificationListenerService
{}
