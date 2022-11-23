// Copyright Snaps 2022, All Rights Reserved.

#include "Engine.h"
#include "Application.h"
#include "Renderer.h"

bool FEngine::Initialize()
{
	// Create renderer.
	Renderer = std::make_unique<FRenderer>();
	return true;
}

void FEngine::Tick()
{
	if(Renderer.get()) // Draw the render texture.
	{
		Renderer.get()->Draw();
	}
}