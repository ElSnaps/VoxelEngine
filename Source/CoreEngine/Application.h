// Copyright Snaps 2022, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FEngine;
struct SDL_Window;

namespace AppSettings
{
	static int32 	WindowWidth 	= 500;
	static int32 	WindowHeight 	= 500;
	static bool		VSync 			= true;
}

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

	static int 	Initialize(HINSTANCE hInstance, int32 nCmdShow);
	void 		Shutdown();

	static FApp* Get()
	{
		return FApp::AppSingleton.get();
	}

	void SetWindowTexture();

	SDL_Window* Window = nullptr;

private:

	static std::unique_ptr<FApp> AppSingleton;
};