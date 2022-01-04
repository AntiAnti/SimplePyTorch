// VR IK Body Plugin
// (c) Yuri N Kalinin, 2021-2022, ykasczc@gmail.com. All right reserved.

#include "SimplePyTorch.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSimplePyTorchModule"

void FSimplePyTorchModule::StartupModule()
{
	// Init DLL from a Path
	FString FilePath;
	const FString szBinaries = TEXT("Binaries");
	const FString szPlatform = TEXT("Win64");

#if WITH_EDITOR
	auto ThisPlugin = IPluginManager::Get().FindPlugin(TEXT("SimplePyTorch"));
	if (ThisPlugin.IsValid())
	{
		FilePath = FPaths::ConvertRelativePathToFull(ThisPlugin->GetBaseDir());
		FilePath = FilePath / TEXT("Source/ThirdParty/pytorch") / szBinaries / szPlatform;
	}
	else
	{
		FilePath = FPaths::ProjectDir() / TEXT("Binaries/ThirdParty/PyTorch");
	}
#else
	FilePath = FPaths::ProjectDir() / TEXT("Binaries/ThirdParty/PyTorch");
#endif
	FPlatformProcess::PushDllDirectory(*FilePath);
	FilePath = FilePath / TEXT("torchscript_wrapper.dll");

	WrapperDllHandle = NULL;
	bDllLoaded = false;
	
#if PLATFORM_WINDOWS
	if (FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("SimplePyTorchModule: Loading torch wrapper from %s"), *FilePath);

		WrapperDllHandle = FPlatformProcess::GetDllHandle(*FilePath);

		if (WrapperDllHandle != NULL)
		{
			FuncTSW_LoadScriptModel = (__TSW_LoadScriptModel)FPlatformProcess::GetDllExport(WrapperDllHandle, TEXT("TSW_LoadScriptModel"));
			if (FuncTSW_LoadScriptModel == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("SimplePyTorchModule: can't load function __TSW_LoadScriptModel"));
				return;
			}
			FuncTSW_CheckModel = (__TSW_CheckModel)FPlatformProcess::GetDllExport(WrapperDllHandle, TEXT("TSW_CheckModel"));
			if (FuncTSW_CheckModel == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("SimplePyTorchModule: can't load function __TSW_CheckModel"));
				return;
			}
			FuncTSW_Forward1d = (__TSW_Forward1d)FPlatformProcess::GetDllExport(WrapperDllHandle, TEXT("TSW_Forward1d"));
			if (FuncTSW_Forward1d == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("SimplePyTorchModule: can't load function __TSW_Forward1d"));
				return;
			}
			FuncTSW_ForwardTensor = (__TSW_ForwardTensor)FPlatformProcess::GetDllExport(WrapperDllHandle, TEXT("TSW_ForwardTensor"));
			if (FuncTSW_ForwardTensor == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("SimplePyTorchModule: can't load function __TSW_ForwardTensor"));
				return;
			}
			FuncTSW_ForwardPass_Def = (__TSW_ForwardPass_Def)FPlatformProcess::GetDllExport(WrapperDllHandle, TEXT("TSW_ForwardPass_Def"));
			if (FuncTSW_ForwardPass_Def == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("SimplePyTorchModule: can't load function __TSW_ForwardPass_Def"));
				return;
			}
			FuncTSW_Execute_Def = (__TSW_Execute_Def)FPlatformProcess::GetDllExport(WrapperDllHandle, TEXT("TSW_Execute_Def"));
			if (FuncTSW_Execute_Def == NULL)
			{
				UE_LOG(LogTemp, Error, TEXT("SimplePyTorchModule: can't load function __TSW_Execute_Def"));
				return;
			}
			
			bDllLoaded = true;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FSimplePyTorchModule. Can't find dll: %s"), *FilePath);
	}
#endif
}

void FSimplePyTorchModule::ShutdownModule()
{
	if (WrapperDllHandle != NULL)
	{
		FPlatformProcess::FreeDllHandle(WrapperDllHandle);
		WrapperDllHandle = NULL;
		bDllLoaded = false;
		FuncTSW_LoadScriptModel = NULL;
		FuncTSW_CheckModel = NULL;
		FuncTSW_Forward1d = NULL;
		FuncTSW_ForwardTensor = NULL;
		FuncTSW_ForwardPass_Def = NULL;
		FuncTSW_Execute_Def = NULL;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSimplePyTorchModule, SimplePyTorch)