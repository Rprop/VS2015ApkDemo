/*
 *	
 *	@author : rrrfff@foxmail.com
 *
 */
#pragma once
#include <dlfcn.h>

#define __LIBC_WEAK__             extern __attribute__((weak))
#define __LIBC_INLINE__           __attribute__((always_inline))
#define NOP                       void(0)
#define UNREFERENCED_FUNCTION(f)  &f ? NOP : NOP
#define LIKELY(exp)               (__builtin_expect((exp) != 0, true))
#define UNLIKELY(exp)             (__builtin_expect((exp) != 0, false))
#define JImport(e, c, f, s)       jmethodID f = e->GetMethodID(c, #f, s)
#define JImportAny(e, c, n, f, s) jmethodID n = e->GetMethodID(c, f, s)
#define PImport(so, symbol)       __typeof__(&symbol) sys_##symbol = PInvoke(so, symbol)
#define PLoad(lib)                ::dlopen(lib, RTLD_LAZY)
#define PFree(handle)             ::dlclose(handle)
#define PError                    ::dlerror
#ifdef __cplusplus
# define PInvoke(so, symbol)    __PInvoke<__typeof__(symbol)>(so, SYMBOL_##symbol, #symbol)
template <typename func> __LIBC_HIDDEN__ __LIBC_INLINE__ func *__PInvoke(void *handle, const char *mangled_symbol, const char *symbol)
{
	void *sym = ::dlsym(handle, mangled_symbol);
	if (UNLIKELY(sym == NULL)) sym = ::dlsym(handle, symbol);
	return reinterpret_cast<func *>(sym);
}
#else
# define PInvoke(so, symbol)    (__typeof__(&symbol))__PInvoke(so, SYMBOL_##symbol, #symbol)
__LIBC_HIDDEN__ __LIBC_WEAK__ void *__PInvoke(void *handle, const char *mangled_symbol, const char *symbol)
{
	void *sym = ::dlsym(handle, mangled_symbol);
	if (UNLIKELY(sym == NULL)) sym = ::dlsym(handle, symbol);
	return sym;
}
#endif // __cplusplus