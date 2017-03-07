#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, __FUNCTION__, __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, __FUNCTION__, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, __FUNCTION__, __VA_ARGS__))
#define LOGF(...) ((void)__android_log_print(ANDROID_LOG_FATAL, __FUNCTION__, __VA_ARGS__))

//-------------------------------------------------------------------------

static void native_onClick(JNIEnv *env, jobject thiz, jobject view)
{
	// thiz = Landroid/app/NativeActivity;
	static bool clicked = false;
	clicked = !clicked;

	jclass viewCls     = env->GetObjectClass(view);
	jmethodID setText  = env->GetMethodID(viewCls, "setText", "(Ljava/lang/CharSequence;)V");
	jstring stopString = env->NewStringUTF(clicked ? "stop" : "start");
	env->CallNonvirtualVoidMethod(view, viewCls, setText, stopString);
	env->DeleteLocalRef(viewCls);

	if (clicked) {
		jclass vpnsvrcls  = env->FindClass("android/net/VpnService");
		jmethodID prepare = env->GetStaticMethodID(vpnsvrcls, "prepare", "(Landroid/content/Context;)Landroid/content/Intent;");
		if (prepare == NULL) {
			env->ExceptionDescribe();
			return env->ExceptionClear();
		} //if

		jclass activityCls = NULL; // android.app.NativeActivity
		jobject intent;
__prepare_again:
		intent = env->CallStaticObjectMethod(vpnsvrcls, prepare, thiz);
		if (intent != NULL) {
			if (activityCls == NULL) {
				activityCls = env->GetObjectClass(thiz);
				JImport(env, activityCls, startActivityForResult, "(Landroid/content/Intent;I)V");
				env->CallVoidMethod(thiz, startActivityForResult, intent, 0);
				goto __prepare_again;
			} else {
				LOGD("user cancel or exception occurred!");
			} //if
		} else {
			LOGD("prepared");

			JImportAny(env, vpnsvrcls, vinit, "<init>", "()V");
			jobject vpnsvr = env->NewObject(vpnsvrcls, vinit);

			jclass buildercls = env->FindClass("android/net/VpnService$Builder");
			JImport(env, buildercls, addAddress, "(Ljava/lang/String;I)Landroid/net/VpnService$Builder;");
			JImport(env, buildercls, addDnsServer, "(Ljava/lang/String;)Landroid/net/VpnService$Builder;");
			JImport(env, buildercls, addRoute, "(Ljava/lang/String;I)Landroid/net/VpnService$Builder;");
			JImport(env, buildercls, establish, "()Landroid/os/ParcelFileDescriptor;");
			JImport(env, buildercls, setMtu, "(I)Landroid/net/VpnService$Builder;");
			JImport(env, buildercls, setSession, "(Ljava/lang/String;)Landroid/net/VpnService$Builder;");
			JImportAny(env, buildercls, init, "<init>", "(Landroid/net/VpnService;)V");
			jobject builder = env->NewObject(buildercls, init, vpnsvr);
			env->CallNonvirtualObjectMethod(builder, buildercls, setMtu, 65535);
			env->CallNonvirtualObjectMethod(builder, buildercls, addAddress, env->NewStringUTF("10.0.8.8"), 32);
			jobject pfd = env->CallNonvirtualObjectMethod(builder, buildercls, establish);
		} //if
	} //if
}

//-------------------------------------------------------------------------

static void dump_class_methods(void *dvm, JNIEnv *env, const char *clsname)
{
	Thread *self = PInvoke(dvm, dvmThreadSelf)();
	jclass cls   = env->FindClass(clsname);
	if (cls == NULL) {
		env->ExceptionDescribe();
		return env->ExceptionClear();
	} //if

	PImport(dvm, dexProtoGetShorty);
	PImport(dvm, dexStringCacheInit);
	PImport(dvm, dexStringCacheRelease);
	PImport(dvm, dexProtoGetMethodDescriptor);
	ClassObject *clsobj = static_cast<ClassObject *>(PInvoke(dvm, dvmDecodeIndirectRef)(self, cls));
	DexStringCache sc;
	for (int k = 0; k < clsobj->virtualMethodCount; ++k) {
		Method *md = &clsobj->virtualMethods[k];	
		sys_dexStringCacheInit(&sc);
		LOGD("virtualMethod %s, \"%s\" %s %s", 
			 md->name, 
			 sys_dexProtoGetMethodDescriptor(&md->prototype, &sc), 
			 sys_dexProtoGetShorty(&md->prototype),
			 (md->accessFlags & ACC_NATIVE) != 0 ? "native" : "instance");
		sys_dexStringCacheRelease(&sc);
	}

	for (int k = 0; k < clsobj->directMethodCount; ++k) {
		Method *md = &clsobj->directMethods[k];
		sys_dexStringCacheInit(&sc);
		LOGD("directMethod %s, \"%s\" %s %s",
			 md->name,
			 sys_dexProtoGetMethodDescriptor(&md->prototype, &sc),
			 sys_dexProtoGetShorty(&md->prototype),
			 (md->accessFlags & ACC_NATIVE) != 0 ? "native" : "instance");
		sys_dexStringCacheRelease(&sc);
	}

	env->DeleteLocalRef(cls);
}

//-------------------------------------------------------------------------

static void app_implement_OnClickListener(jclass activityCls, JNIEnv *env)
{
	void *dvm = PLoad("libdvm.so");
	if (dvm == NULL) {
		return LOGD("%s", PError());
	} //if

	Thread *self = PInvoke(dvm, dvmThreadSelf)();
	PImport(dvm, dvmDecodeIndirectRef);
	
	UNREFERENCED_FUNCTION(dump_class_methods);
//	dump_class_methods(dvm, env, "android/net/VpnService");
//	dump_class_methods(dvm, env, "android/net/VpnService$Builder");
//	dump_class_methods(dvm, env, "com/android/server/connectivity/Vpn");

	ClassObject *activityClsObj = static_cast<ClassObject *>(sys_dvmDecodeIndirectRef(self, activityCls));
	jclass listenerCls          = env->FindClass("android/view/View$OnClickListener");
	ClassObject *listenerClsObj = static_cast<ClassObject *>(sys_dvmDecodeIndirectRef(self, listenerCls));
	Method *abstract_onClick    = &listenerClsObj->virtualMethods[0];
	env->DeleteLocalRef(listenerCls);

	// replaces interface
	for (int i = 0; i < activityClsObj->interfaceCount; ++i)
	{
		ClassObject *cs = activityClsObj->interfaces[i];
		if (strcmp(cs->descriptor, "Landroid/view/SurfaceHolder$Callback2;") == 0) {
			activityClsObj->interfaces[i] = listenerClsObj;
			break;
		} //if
//		LOGD("interfaces -- %s", cs->descriptor);
	} //for

	// replaces method
	for (int k = 0; k < activityClsObj->virtualMethodCount; ++k) {
		Method *md = &activityClsObj->virtualMethods[k];
		if (strcmp(md->name, "surfaceCreated") == 0) {
			// md == &activityClsObj->vtable[md->methodIndex]
			md->accessFlags   = ACC_PUBLIC | ACC_NATIVE;
			md->registersSize = abstract_onClick->registersSize;
			md->outsSize  = abstract_onClick->outsSize;
			md->insSize   = abstract_onClick->insSize;
			md->name      = abstract_onClick->name;
			md->prototype = abstract_onClick->prototype;
			md->shorty = abstract_onClick->shorty;
			md->insns  = abstract_onClick->insns;
			md->jniArgInfo = md->registersSize;
			md->nativeFunc = NULL;

			// replaces iftable, http://androidxref.com/4.4.4_r1/xref/dalvik/vm/interp/Interp.cpp#1258
			for (int v = 0; v < activityClsObj->iftableCount; ++v) {
				InterfaceEntry *ife = &activityClsObj->iftable[v];
				if (strcmp(ife->clazz->descriptor, "Landroid/view/SurfaceHolder$Callback;") == 0) {
					ife->clazz = listenerClsObj;
					ife->methodIndexArray[abstract_onClick->methodIndex] = md->methodIndex;
					break;
				} //if
//				LOGD("iftable -- %s -- %d", ife->clazz->descriptor, *ife->methodIndexArray);
			} //for
			break;
		} //if
//		LOGD("virtualMethods -- %s", md->name);
	} //for

	// registers native callbacks
	JNINativeMethod gMethods[] = {
		{ "onClick", "(Landroid/view/View;)V", reinterpret_cast<void *>(native_onClick) }
	};
	env->RegisterNatives(activityCls, gMethods, NELEM(gMethods));

	PFree(dvm);
}

//-------------------------------------------------------------------------

static void app_recovery_java_activity(jobject activity, jclass superClass, JNIEnv *env)
{
	// getWindow()
	jmethodID getWindow = env->GetMethodID(superClass, "getWindow", "()Landroid/view/Window;");
	jobject window      = env->CallNonvirtualObjectMethod(activity, superClass, getWindow);
	jclass windowCls    = env->GetObjectClass(window);

	// getWindow().takeSurface(null)
	jmethodID takeSurface = env->GetMethodID(windowCls, "takeSurface", "(Landroid/view/SurfaceHolder$Callback2;)V");
	env->CallNonvirtualVoidMethod(window, windowCls, takeSurface, NULL);

	// getWindow().takeInputQueue(null)
	jmethodID takeInputQueue = env->GetMethodID(windowCls, "takeInputQueue", "(Landroid/view/InputQueue$Callback;)V");
	env->CallNonvirtualVoidMethod(window, windowCls, takeInputQueue, NULL);

	env->DeleteLocalRef(windowCls);
	env->DeleteLocalRef(window);
}

//-------------------------------------------------------------------------

__LIBC_HIDDEN__ JNICALL void app_init_activity(ANativeActivity *nativeActivity)
{
	JNIEnv *env        = nativeActivity->env;
	jobject activity   = nativeActivity->clazz;
	jclass activityCls = env->GetObjectClass(activity);   // android.app.NativeActivity
	jclass superClass  = env->GetSuperclass(activityCls); // android.app.Activity

	app_recovery_java_activity(activity, superClass, env);

	// super.onCreate(savedInstanceState)
	jmethodID superOnCreate = env->GetMethodID(superClass, "onCreate", "(Landroid/os/Bundle;)V");
	env->CallNonvirtualVoidMethod(activity, superClass, superOnCreate, NULL);

	// setContentView(R.layout.activity_main)
	jmethodID setContentView = env->GetMethodID(superClass, "setContentView", "(I)V");
	env->CallNonvirtualVoidMethod(activity, superClass, setContentView, 0x7f020000);

	// btnStart = findViewById(R.id.buttonStart)
	jmethodID findViewById = env->GetMethodID(superClass, "findViewById", "(I)Landroid/view/View;");
	jobject btnStart       = env->CallNonvirtualObjectMethod(activity, superClass, findViewById, 0x7f040000);

	env->DeleteLocalRef(superClass);
	app_implement_OnClickListener(activityCls, env);

	// btnStart.setOnClickListener(...)
	jclass btnCls                = env->GetObjectClass(btnStart);
	jmethodID setOnClickListener = env->GetMethodID(btnCls, "setOnClickListener", "(Landroid/view/View$OnClickListener;)V");
	env->CallNonvirtualVoidMethod(btnStart, btnCls, setOnClickListener, activity);

	env->DeleteLocalRef(btnCls);
}
