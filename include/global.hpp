#ifndef __CU_GLOBAL__
#define __ACU_GLOBAL__

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	#define DECL_EXPORT __declspec(dllexport)
	#define DECL_IMPORT __declspec(dllimport)
#else
	#define DECL_EXPORT __attribute__((visibility("default")))
	#define DECL_IMPORT __attribute__((visibility("default")))
#endif

#if defined(ACU_SHARED_LIBRARY_EXPORT)
	#define ACUSHARED_EXPORT DECL_EXPORT
#elif defined(ACU_SHARED_LIBRARY_IMPORT)
	#define ACUSHARED_EXPORT DECL_IMPORT
#else
	#define ACUSHARED_EXPORT
#endif

#endif // __ACU_GLOBAL__
