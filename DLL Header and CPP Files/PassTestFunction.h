#ifdef PASSTESTFUNCTION_EXPORTS
#define PASSTESTFUNCTION_API __declspec(dllexport)
#else
#define PASSTESTFUNCTION_API __declspec(dllimport)
#endif

extern "C" PASSTESTFUNCTION_API bool PassTestFunction();