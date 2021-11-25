// VR IK Body Plugin
// (c) Yuri N Kalinin, 2021, ykasczc@gmail.com. All right reserved.

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
			// my PyTorch wrapper
			RuntimeDependencies.Add("$(BinaryOutputDir)/torchscript_wrapper.dll", Path.Combine(TorchBinariesPath, "torchscript_wrapper.dll"));
			if (!Target.bBuildEditor)
			{
				// Copy DLLs to target packaged project
				RuntimeDependencies.Add("$(BinaryOutputDir)/asmjit.dll", Path.Combine(TorchBinariesPath, "asmjit.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/c10.dll", Path.Combine(TorchBinariesPath, "c10.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/caffe2_detectron_ops.dll", Path.Combine(TorchBinariesPath, "caffe2_detectron_ops.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/caffe2_module_test_dynamic.dll", Path.Combine(TorchBinariesPath, "caffe2_module_test_dynamic.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/fbgemm.dll", Path.Combine(TorchBinariesPath, "fbgemm.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/fbjni.dll", Path.Combine(TorchBinariesPath, "fbjni.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/libiomp5md.dll", Path.Combine(TorchBinariesPath, "libiomp5md.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/libiompstubs5md.dll", Path.Combine(TorchBinariesPath, "libiompstubs5md.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/pytorch_jni.dll", Path.Combine(TorchBinariesPath, "pytorch_jni.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/torch.dll", Path.Combine(TorchBinariesPath, "torch.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/torch_cpu.dll", Path.Combine(TorchBinariesPath, "torch_cpu.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/torch_global_deps.dll", Path.Combine(TorchBinariesPath, "torch_global_deps.dll"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/uv.dll", Path.Combine(TorchBinariesPath, "uv.dll"));
				// licenses
				RuntimeDependencies.Add("$(BinaryOutputDir)/LICENSE.txt", Path.Combine(TorchPath, "LICENSE.txt"));
				RuntimeDependencies.Add("$(BinaryOutputDir)/NOTICE.txt", Path.Combine(TorchPath, "NOTICE.txt"));
			}
		}
	}
}
