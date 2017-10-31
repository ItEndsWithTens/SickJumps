#ifndef SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
#define SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E



#include <vector>

#include <string>

#include "avisynth.h"

#include "SickJumpsCore.h"



class SickJumps : public GenericVideoFilter
{
public:
	enum
	{
		MODE_LINEAR = 0,
		MODE_SPLINE
	};

  SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier,
		SFLOAT _inSeconds, SFLOAT _outSeconds, int _mode, std::string _scriptVariable, IScriptEnvironment* env);
  ~SickJumps();

	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	int __stdcall SetCacheHints(int cachehints, int frame_range);

	int mode;

	int firstFrame;
	int lastFrame;

	SFLOAT startMultiplier;
	SFLOAT fullMultiplier;

	SFLOAT upSeconds;
	SFLOAT downSeconds;

	std::string scriptVariable;

private:
	bool setScriptVariable;

	SickJumpsCore core;
};



#endif // SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
