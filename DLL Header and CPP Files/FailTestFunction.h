#ifdef FAILTESTFUNCTION_EXPORTS
#define FAILTESTFUNCTION_API __declspec(dllexport)
#else
#define FAILTESTFUNCTION_API __declspec(dllimport)
#endif

extern "C" FAILTESTFUNCTION_API bool FailTestFunction();