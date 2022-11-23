// Copyright Snaps 2022, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FRenderer;

/*
	Engine is the base level object for the entire engine. This is the actual
	running instance of the engine owned by App
*/
class FEngine
{
public:

	bool Initialize();
	void Tick();

private:

	std::shared_ptr<FRenderer> Renderer;
};