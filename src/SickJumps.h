#ifndef SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
#define SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E



#include <vector>

#include <string>

#include <avisynth.h>



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

	int mode;

	int firstFrame;
	int lastFrame;

	SFLOAT startMultiplier;
	SFLOAT fullMultiplier;

	SFLOAT upSeconds;
	SFLOAT downSeconds;

	std::string scriptVariable;

private:
	int CalculateRampInputFrames(int _firstInputFrame, int _totalOutputFrames, SFLOAT _startMultiplier, SFLOAT _endMultiplier, IScriptEnvironment* env);

	int rampUpFirstOutputFrame;
	int rampUpLastOutputFrame;
	int rampUpFirstInputFrame;
	int rampUpLastInputFrame;
	
	int fullSpeedFirstOutputFrame;
	int fullSpeedLastOutputFrame;
	int fullSpeedFirstInputFrame;
	int fullSpeedLastInputFrame; 
	
	int rampDownFirstOutputFrame;
	int rampDownLastOutputFrame;
	int rampDownFirstInputFrame;
	int rampDownLastInputFrame;

	__int64 rampUpFirstOutputSample;
	__int64 rampUpLastOutputSample;
	__int64 rampUpFirstInputSample;
	__int64 rampUpLastInputSample;

	__int64 fullSpeedFirstOutputSample;
	__int64 fullSpeedLastOutputSample;
	__int64 fullSpeedFirstInputSample;
	__int64 fullSpeedLastInputSample;

	__int64 rampDownFirstOutputSample;
	__int64 rampDownLastOutputSample;
	__int64 rampDownFirstInputSample;
	__int64 rampDownLastInputSample;

	bool setScriptVariable;
};



double GetCurrentMultiplier(__int64 n, __int64 first, __int64 last, SFLOAT startMultiplier, SFLOAT fullMultiplier, int mode, IScriptEnvironment * env);
double ScaleToRange(__int64 value, __int64 inMin, __int64 inMax, double outMin, double outMax);



#endif // SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
