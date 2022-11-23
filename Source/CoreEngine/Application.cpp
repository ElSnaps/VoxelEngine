// Copyright Snaps 2022, All Rights Reserved.

#include "Application.h"
#include "Engine.h"
#include "SDL.h"

std::unique_ptr<FApp> FApp::AppSingleton;

int FApp::Initialize(HINSTANCE hInstance, int32 nCmdShow)
{
	SDL_Event LatestEvent;

	// Create the main app singleton
	FApp::AppSingleton = std::make_unique<FApp>();
	FApp* App = FApp::Get();
	App->AppState = EAppState::Starting;

	// Initialize SDL & create blank SDL window
	SDL_Init(SDL_INIT_VIDEO);
	App->Window = SDL_CreateWindow(
		"Voxel Engine", 						// Window title
		SDL_WINDOWPOS_UNDEFINED, 				// ScreenPos X (Don't care)
		SDL_WINDOWPOS_UNDEFINED, 				// ScreenPos Y (Don't care)
		AppDefaults::WindowWidth, 				// Window width in pixels
		AppDefaults::WindowHeight, 				// Window height in pixels
		(SDL_WindowFlags)(SDL_WINDOW_RESIZABLE) // Set window flags
	);

	// Create engine and initialize. Process will exit if engine init fails.
	App->GEngine = std::make_shared<FEngine>();
	if(!App->GEngine.get()->Initialize())
	{
		return 1; // Quit process with error code 1.
	}

	App->AppState = EAppState::Running;
	while(App->AppState != EAppState::Exiting)
	{
		App->GEngine.get()->Tick(); // Tick our engine.

		// Poll for SDL window events
		while(SDL_PollEvent(&LatestEvent) != 0)
		{
			// Process window quit events like close and alt-f4
			if(LatestEvent.type == SDL_QUIT)
			{
				App->AppState = EAppState::Exiting;
			}
		}
	}
	App->OnShutdown(); // Run all shutdown prereqs & cleanup.
	return 0;
}

void FApp::OnShutdown()
{
	// Kill the active SDL window.
	SDL_DestroyWindow(Window);
}

void FApp::SetRenderTexture()
{

}