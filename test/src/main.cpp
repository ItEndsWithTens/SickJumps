#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"

#include <Windows.h>

#include "avisynth.h"



const AVS_Linkage* AVS_linkage = 0;



IScriptEnvironment* env = 0;



int main(int argc, char* argv[])
{
	typedef IScriptEnvironment* (__stdcall *CSE)(int);

	const char* libname = "avisynth";

	HMODULE lib = LoadLibrary(libname);
	if (!lib)
	{
		std::cerr << "Couldn't load Avisynth!" << std::endl;
		return -1;
	}

	CSE makeEnv = (CSE)GetProcAddress(lib, "CreateScriptEnvironment");
	if (!makeEnv)
	{
		std::cerr << "Couldn't find CreateScriptEnvironment function!" << std::endl;
		return -1;
	}

	try
	{
		env = makeEnv(AVISYNTH_INTERFACE_VERSION);
	}
	catch (AvisynthError& e)
	{
		std::cerr << "Avisynth error: " << e.msg;
		return -1;
	}
	if (!env)
	{
		std::cerr << "Couldn't create script environment!" << std::endl;
		return -1;
	}

	AVS_linkage = env->GetAVSLinkage();

	try
	{

		const char* ver = env->Invoke("VersionString", AVSValue(0, 0)).AsString();
		std::cout << "Plugin host: " << ver << std::endl << std::endl;

	}
	catch (AvisynthError& err)
	{

		std::cout << err.msg << std::endl;
		return -1;

	}

	if (!env->FunctionExists("SickJumps"))
	{
		std::cerr << "Couldn't find SickJumps function!" << std::endl;
		return -1;
	}

	int result = Catch::Session().run(argc, argv);

	if (env)
	{
		// Requires AVISYNTH_INTERFACE_VERSION >= 5; that's an enum, and can't be
		// checked by the preprocessor, but then a branch won't work either since
		// DeleteScriptEnvironment doesn't exist before interface version 5, and this
		// code won't even compile. Since this plugin and its tests are both aimed at
		// Avisynth 2.6 and onward, though, just assume the method is present and
		// leave the build errors to those who insist on Avisynth 2.5.
		env->DeleteScriptEnvironment();
	}

	if (lib)
	{
		FreeLibrary(lib);
	}

	return result;
}
