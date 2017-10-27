#include <string>
#include <tuple>

class __declspec(dllexport) SickJumpsCore
{
public:
	SickJumpsCore();
	SickJumpsCore(int frameCount, int firstFrame, int lastFrame, double fps,
		double upSeconds, double downSeconds, double startMultiplier, double endMultiplier,
		__int64 audioSamplesPerFrame, int _mode);
	~SickJumpsCore();

	__int64 GetAdjustedSampleNumber(__int64 n);
	std::tuple<int, double, std::string> GetAdjustedFrameProperties(int n);

	int mode;

	double startMultiplier;
	double fullMultiplier;

	double upSeconds;
	double downSeconds;

	int originalFrameCount;
	int adjustedFrameCount;

	__int64 originalSampleCount;
	__int64 adjustedSampleCount;

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

	int afterFirstInputFrame;

	__int64 rampUpFirstOutputSample;
	__int64 rampUpLastOutputSample;
	__int64 rampUpFirstInputSample;

	__int64 fullSpeedFirstOutputSample;
	__int64 fullSpeedLastOutputSample;
	__int64 fullSpeedFirstInputSample;

	__int64 rampDownFirstOutputSample;
	__int64 rampDownLastOutputSample;
	__int64 rampDownLastInputSample;

	__int64 afterFirstInputSample;
};

int CalculateRampInputFrames(int firstInputFrame, int totalOutputFrames, double startMultiplier, double endMultiplier, int mode);
__int64 CalculateRampInputSamples(__int64 firstInputSample, __int64 totalOutputSamples, double startMultiplier, double endMultiplier, int mode); 
double GetCurrentMultiplier(__int64 n, __int64 first, __int64 last, double startMultiplier, double fullMultiplier, int mode);
double ScaleToRange(__int64 value, __int64 inMin, __int64 inMax, double outMin, double outMax);
