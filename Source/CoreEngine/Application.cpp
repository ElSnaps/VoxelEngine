// Copyright Snaps 2022, All Rights Reserved.

#include "Application.h"
#include "Engine.h"

std::unique_ptr<FApp> FApp::AppSingleton;

void FApp::Initialize()
{
	FApp::AppSingleton = std::make_unique<FApp>();
	FApp* AppInst = FApp::Get();

	AppInst->AppState = EAppState::Starting;

	// Do app setup here like create window ect.

	// Create Engine
	AppInst->GEngine = std::make_shared<FEngine>();
	AppInst->GEngine.get()->Initialize();

	AppInst->AppState = EAppState::Running;
	while(AppInst->AppState != EAppState::Exiting)
	{
	
	}
}