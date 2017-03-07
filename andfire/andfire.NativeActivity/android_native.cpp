#define JAVA_UI   1
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "android_native", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "android_native", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "android_native", __VA_ARGS__))

//-------------------------------------------------------------------------

extern __LIBC_HIDDEN__ JNICALL void app_init_activity(ANativeActivity *activity);

//-------------------------------------------------------------------------

static void onStart(ANativeActivity *activity) 
{
	LOGD("%s: %p", __FUNCTION__, activity);

	AConfiguration *config = AConfiguration_new();
	AConfiguration_fromAssetManager(config, activity->assetManager);
	LOGD("sdk = %d", AConfiguration_getSdkVersion(config));
	AConfiguration_delete(config);

	app_init_activity(activity);
}

//-------------------------------------------------------------------------

static void onDestroy(ANativeActivity *activity)
{
	LOGD("%s: %p", __FUNCTION__, activity);
}

//-------------------------------------------------------------------------

static void onResume(ANativeActivity *activity) 
{
	LOGD("%s: %p", __FUNCTION__, activity);
}

//-------------------------------------------------------------------------

static void *onSaveInstanceState(ANativeActivity *activity, size_t *outLen) 
{
	LOGD("%s: %p", __FUNCTION__, activity);

	*outLen = 0;
    return NULL;
}

//-------------------------------------------------------------------------

static void onPause(ANativeActivity *activity) 
{
	LOGD("%s: %p", __FUNCTION__, activity);
}

//-------------------------------------------------------------------------

static void onStop(ANativeActivity *activity)
{
	LOGD("%s: %p", __FUNCTION__, activity);
}

//-------------------------------------------------------------------------

static void onConfigurationChanged(ANativeActivity *activity)
{
	LOGD("%s: %p", __FUNCTION__, activity);
}

//-------------------------------------------------------------------------

static void onLowMemory(ANativeActivity *activity) 
{
	LOGD("%s: %p", __FUNCTION__, activity);
}

//-------------------------------------------------------------------------

static void onWindowFocusChanged(ANativeActivity *activity, int focused)
{
	LOGD("%s: %p -- %d", __FUNCTION__, activity, focused);
}

//-------------------------------------------------------------------------

static void onContentRectChanged(ANativeActivity* activity, const ARect* rect)
{
	LOGD("%s: %p -- %p", __FUNCTION__, activity, rect);
}

//-------------------------------------------------------------------------

#if !JAVA_UI

static void onNativeWindowResized(ANativeActivity* activity, ANativeWindow* window)
{
	LOGD("%s: %p -- %p", __FUNCTION__, activity, window);
}

//-------------------------------------------------------------------------

static void onNativeWindowRedrawNeeded(ANativeActivity* activity, ANativeWindow* window)
{
	LOGD("%s: %p -- %p", __FUNCTION__, activity, window);
}

//-------------------------------------------------------------------------

static void onNativeWindowCreated(ANativeActivity *activity, ANativeWindow *window)
{
    LOGD("%s: %p -- %p", __FUNCTION__, activity, window);
}

//-------------------------------------------------------------------------

static void onNativeWindowDestroyed(ANativeActivity *activity, ANativeWindow *window)
{
	LOGD("%s: %p -- %p", __FUNCTION__, activity, window);
}

//-------------------------------------------------------------------------

static void *android_input_events(AInputQueue *queue)
{
	AInputEvent *ev;
__loop_start:
	while (AInputQueue_getEvent(queue, &ev) >= 0) {
		if (AInputQueue_preDispatchEvent(queue, ev) == 0) {
			LOGD("%s: preDispatchEvent -- %u", __FUNCTION__, AInputEvent_getType(ev));
			AInputQueue_finishEvent(queue, ev, 0);
		} else {
			LOGD("%s: !preDispatchEvent -- %u", __FUNCTION__, AInputEvent_getType(ev));
			AInputQueue_finishEvent(queue, ev, 1);
		} //if	
	} //if
	goto __loop_start;
}

//-------------------------------------------------------------------------

static void onInputQueueCreated(ANativeActivity *activity, AInputQueue *queue)
{
    LOGD("%s: %p -- %p", __FUNCTION__, activity, queue);

	pthread_t thread;
	pthread_create(&thread, NULL, reinterpret_cast<void *(*)(void *)>(android_input_events), queue);
	pthread_detach(thread);
}

//-------------------------------------------------------------------------

static void onInputQueueDestroyed(ANativeActivity *activity, AInputQueue *queue)
{
	LOGD("%s: %p -- %p", __FUNCTION__, activity, queue);
}

#endif // !JAVA_UI

//-------------------------------------------------------------------------

void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize)
{
    LOGD("Creating: %p", activity);
	activity->instance               = NULL;
	activity->callbacks->onStart     = onStart;
    activity->callbacks->onDestroy   = onDestroy;
    activity->callbacks->onResume    = onResume;
    activity->callbacks->onPause     = onPause;
    activity->callbacks->onStop      = onStop;
	activity->callbacks->onLowMemory = onLowMemory;
	activity->callbacks->onSaveInstanceState        = onSaveInstanceState;
    activity->callbacks->onConfigurationChanged     = onConfigurationChanged;
	activity->callbacks->onWindowFocusChanged       = onWindowFocusChanged;
	activity->callbacks->onContentRectChanged       = onContentRectChanged;
#if !JAVA_UI
	activity->callbacks->onNativeWindowResized      = onNativeWindowResized;
	activity->callbacks->onNativeWindowRedrawNeeded = onNativeWindowRedrawNeeded;
    activity->callbacks->onNativeWindowCreated      = onNativeWindowCreated;
    activity->callbacks->onNativeWindowDestroyed    = onNativeWindowDestroyed;
    activity->callbacks->onInputQueueCreated        = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed      = onInputQueueDestroyed;
#endif // !JAVA_UI
}
