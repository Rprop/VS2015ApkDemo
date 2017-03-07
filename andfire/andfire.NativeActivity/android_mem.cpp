int ret = mprotect(reinterpret_cast<const void *>(ALIGN_DOWN_TO_PAGE_SIZE(cls->interfaces)),
				   SYSTEM_PAGE_SIZE,
				   PROT_READ | PROT_WRITE);
if (ret != 0) {
	LOGE("mprotect failed %d", errno);
} //if

		Object *loader  = PInvoke(dvm, dvmGetSystemClassLoader)();
		cls->interfaces = reinterpret_cast<ClassObject **>(PInvoke(dvm, dvmLinearRealloc)(loader, cls->interfaces, (cls->interfaceCount + 1) * sizeof(ClassObject *)));
//		NOP, PInvoke(dvm, dvmLinearSetReadWrite)(loader, cls->interfaces);
		NOP, PInvoke(dvm, dvmReleaseTrackedAlloc)(loader, self);