// Copyright Snaps 2022, All Rights Reserved.

#include "Application.h"
#include "Engine.h"

std::unique_ptr<FApp> FApp::AppSingleton;

int FApp::Initialize(HINSTANCE hInstance, int32 nCmdShow)
{
	FApp::AppSingleton = std::make_unique<FApp>();
	FApp* App = FApp::Get();

	App->AppState = EAppState::Starting;

	// Do app setup here like create window ect.

	// Create Engine and Initialize. Process will exit if Engine init fails.
	App->GEngine = std::make_shared<FEngine>();
	if(!App->GEngine.get()->Initialize())
	{
		return 1;
	}

	App->AppState = EAppState::Running;
	while(App->AppState != EAppState::Exiting)
	{
		App->GEngine.get()->Tick();
	}
	return 0;
}