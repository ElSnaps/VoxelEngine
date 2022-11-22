// Copyright Snaps 2022. All Rights Reserved.

#include "CoreMinimal.h"
#include "Application.h"

/*
	Main handles all our entry points for launching into our application on various
	platform architectures, Aka windows or linux entry points will exist in here.
*/

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	return FApp::Initialize(hInstance, nCmdShow);
}