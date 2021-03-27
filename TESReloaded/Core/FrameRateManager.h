#pragma once

enum FrameRateRequestType {
	None,
	Up,
	Down,
};

class FrameRateManager { // Never disposed
public:
	FrameRateManager();

	void				Set();
	void				SetFrameTime(double CurrentFrameTime);
	bool				IsOutGrid(NiAVObject* Object);
	
	int					CurrentFrameRate;
	double				ElapsedTime;

private:
	double				LastFrameTime;
	int					FrameCounter;
	UInt32				GridDistant;
	UInt32				RequestType;

};