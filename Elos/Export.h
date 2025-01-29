#pragma once

#if !defined(_WIN32)
#error Elos is only supported on Windows
#endif


#ifdef _MSC_VER

#pragma warning(disable : 4251) // Using standard library types in our own exported types is okay
#pragma warning(disable : 4275) // Exporting types derived from the standard library is okay

#endif

#if defined(ELOS_EXPORTS)
#define ELOS_EXPORT __declspec(dllexport)
#define ELOS_IMPORT __declspec(dllimport)
#else
#define ELOS_EXPORT
#define ELOS_IMPORT
#endif

#if defined(ELOS_EXPORTS)
#define ELOS_API ELOS_EXPORT
#else
#define ELOS_API ELOS_IMPORT
#endif