#ifdef THROWTESTFUNCTION_EXPORTS
#define THROWTESTFUNCTION_API __declspec(dllexport)
#else
#define THROWTESTFUNCTION_API __declspec(dllimport)
#endif

extern "C" THROWTESTFUNCTION_API bool ThrowTestFunction();