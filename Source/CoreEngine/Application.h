// Copyright Snaps 2022, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <Memory>

class FEngine;

enum class EAppState : uint8_t
{
	None,
	Starting,
	Running,
	Exiting
};

/*
	Application singleton, responsible for the lifetime of the application.
	When application state is set to exiting our process will cleanup and exit.
*/
class FApp
{
public:

	EAppState 					AppState = EAppState::None;
	std::shared_ptr<FEngine> 	GEngine;

	static int Initialize();

	static FApp* Get()
	{
		return FApp::AppSingleton.get();
	}

private:

	static std::unique_ptr<FApp> AppSingleton;
};