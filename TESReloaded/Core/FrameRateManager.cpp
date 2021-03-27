FrameRateManager::FrameRateManager() {

	Logger::Log("Starting the framerate manager...");
	TheFrameRateManager = this;

	FrameCounter = 0;
	CurrentFrameRate = 0;
	LastFrameTime = 0.0;
	ElapsedTime = 0.0;
	GridDistant = *SettingGridDistantCount;

}

void FrameRateManager::Set() {
	
	SettingsMainStruct::FrameRateStruct* FrameRate = &TheSettingManager->SettingsMain.FrameRate;

	if (CurrentFrameRate > 0) {
		if (CurrentFrameRate < FrameRate->Critical) {
			FrameCounter++;
			if (FrameCounter >= FrameRate->Delay) {
				FrameCounter = 0;
				GridDistant = 1;
				*SettingLODFadeOutMultItems = *SettingLODFadeOutMultObjects = 1;
				*SettingLODFadeOutMultActors = 1;
				RequestType = FrameRateRequestType::None;
			}
		}
		else if (CurrentFrameRate < FrameRate->Min) {
			FrameCounter++;
			if (FrameCounter >= FrameRate->Delay) {
				FrameCounter = 0;
				GridDistant = FrameRate->GridMin;
				RequestType = FrameRateRequestType::Down;
			}
		}
		else if (CurrentFrameRate < FrameRate->Average - FrameRate->Gap) {
			FrameCounter++;
			if (FrameCounter >= FrameRate->Delay) {
				FrameCounter = 0;
				GridDistant -= FrameRate->GridStep;
				if (GridDistant < FrameRate->GridMin) GridDistant = FrameRate->GridMin;
				RequestType = FrameRateRequestType::Down;
			}
		}
		else if (CurrentFrameRate > FrameRate->Average + FrameRate->Gap) {
			FrameCounter++;
			if (FrameCounter >= FrameRate->Delay) {
				FrameCounter = 0;
				if (*SettingLODFadeOutMultActors == 15.0f && *SettingLODFadeOutMultObjects == 15.0f) {
					GridDistant += FrameRate->GridStep;
					if (GridDistant > *SettingGridDistantCount) GridDistant = *SettingGridDistantCount;
				}
				RequestType = FrameRateRequestType::Up;
			}
		}
		else {
			FrameCounter = 0;
			RequestType = FrameRateRequestType::None;
		}
		if (RequestType == FrameRateRequestType::Down) {
			if (GridDistant == FrameRate->GridMin) {
				if (*SettingLODFadeOutMultObjects > FrameRate->FadeMinObjects) {
					*SettingLODFadeOutMultItems = *SettingLODFadeOutMultObjects = *SettingLODFadeOutMultObjects - FrameRate->FadeStep;
					if (*SettingLODFadeOutMultObjects < FrameRate->FadeMinObjects)
						*SettingLODFadeOutMultItems = *SettingLODFadeOutMultObjects = FrameRate->FadeMinObjects;
				}
				else if (*SettingLODFadeOutMultActors > FrameRate->FadeMinActors) {
					*SettingLODFadeOutMultActors = *SettingLODFadeOutMultActors - FrameRate->FadeStep;
					if (*SettingLODFadeOutMultActors < FrameRate->FadeMinActors)
						*SettingLODFadeOutMultActors = FrameRate->FadeMinActors;
				}
			}
		}
		else if (RequestType == FrameRateRequestType::Up) {
			if (*SettingLODFadeOutMultActors < 15.0f) {
				*SettingLODFadeOutMultActors = *SettingLODFadeOutMultActors + FrameRate->FadeStep;
				if (*SettingLODFadeOutMultActors > 15.0f)
					*SettingLODFadeOutMultActors = 15.0f;
			}
			else if (*SettingLODFadeOutMultObjects < 15.0f) {
				*SettingLODFadeOutMultItems = *SettingLODFadeOutMultObjects = *SettingLODFadeOutMultObjects + FrameRate->FadeStep;
				if (*SettingLODFadeOutMultObjects > 15.0f)
					*SettingLODFadeOutMultItems = *SettingLODFadeOutMultObjects = 15.0f;
			}
		}
	}

}

void FrameRateManager::SetFrameTime(double CurrentFrameTime) {
	
	ElapsedTime = CurrentFrameTime - LastFrameTime;
	LastFrameTime = CurrentFrameTime;
	CurrentFrameRate = 1.0 / ElapsedTime;

}

bool FrameRateManager::IsOutGrid(NiAVObject* Object) {
	
	float Distance = (GridDistant + *SettingGridsToLoad / 2.0f) * 4096.0f;

	if (Object->GetDistance(&Player->pos) > Distance)
		return true;
	else
		return false;

}