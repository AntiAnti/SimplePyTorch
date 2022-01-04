// VR IK Body Plugin
// (c) Yuri N Kalinin, 2021-2022, ykasczc@gmail.com. All right reserved.

using UnrealBuildTool;
using System.IO;

public class SimplePyTorch : ModuleRules
{
	private string TorchPath
	{
		get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../ThirdParty/pytorch")); }
	}
	private string TorchBinariesPath
	{
		get { return Path.GetFullPath(Path.Combine(TorchPath, "Binaries/Win64")); }
	}

	public SimplePyTorch(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivatePCHHeaderFile = "Public/SimplePyTorch.h";
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// LibTorch libraries
			string[] DLLs = new string[]
			{
				"asmjit.dll", "c10.dll", "caffe2_detectron_ops.dll", "caffe2_module_test_dynamic.dll", "fbgemm.dll", "fbjni.dll", "libiomp5md.dll",
				"libiompstubs5md.dll", "pytorch_jni.dll", "torch.dll", "torch_cpu.dll", "torch_global_deps.dll", "uv.dll"
			};

			// copy all DLLs to the packaged build
			if (!Target.bBuildEditor && Target.Type == TargetType.Game)
			{
				string DllTargetDir = "$(ProjectDir)/Binaries/ThirdParty/PyTorch/";
				foreach (string DllName in DLLs)
                {
					PublicDelayLoadDLLs.Add(DllName);
					RuntimeDependencies.Add(Path.Combine(DllTargetDir, DllName), Path.Combine(TorchBinariesPath, DllName));
				}

				// my PyTorch wrapper is loaded dynamically
				RuntimeDependencies.Add(Path.Combine(DllTargetDir, "torchscript_wrapper.dll"), Path.Combine(TorchBinariesPath, "torchscript_wrapper.dll"));
				// licenses
				RuntimeDependencies.Add(Path.Combine(DllTargetDir, "LICENSE.txt"), Path.Combine(TorchPath, "LICENSE.txt"), StagedFileType.NonUFS);
				RuntimeDependencies.Add(Path.Combine(DllTargetDir, "NOTICE.txt"), Path.Combine(TorchPath, "NOTICE.txt"), StagedFileType.NonUFS);
			}
		}
	}
}
