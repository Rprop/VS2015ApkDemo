#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

static int32_t app_handle_input(struct android_app* app, AInputEvent* event)
{
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		AMotionEvent_getX(event, 0);
		AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

static void app_handle_cmd(struct android_app* app, int32_t cmd) 
{
	switch (cmd) 
	{
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		app->savedState = malloc(sizeof(int));
		app->savedStateSize = sizeof(int);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (app->window != NULL) {
			JNIEnv *env = app->activity->env;
			app->activity->vm->AttachCurrentThread(&env, NULL);
			//app_init_activity(app, env);
			app->activity->vm->DetachCurrentThread();
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop...
		// This is to avoid consuming battery while not being used.
		break;
	}
}

void android_main(struct android_app* state) 
{
	state->onAppCmd     = app_handle_cmd;
	state->onInputEvent = app_handle_input;

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
	}

	// loop waiting for stuff to do.
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				break;
			}
		}
	}
}
