# SimplePyTorch
UE4 Plugin to execute trained PyTorch modules

------- Packaging -------

1. Download PyTorch C++ distributions: https://pytorch.org/cppdocs/installing.html
2. Copy DLLs from lib directory to SimplePyTorch/Source/ThirdParty/pytorch/Binaries/Win64:
asmjit.dll, c10.dll, caffe2_detectron_ops.dll, caffe2_module_test_dynamic.dll, fbgemm.dll, fbjni.dll, libiomp5md.dll, libiompstubs5md.dll, torch.dll, torch_cpu.dll, torch_global_deps.dll, uv.dll.
3. Copy a whole plugin to [your UE4 project]/Plugins directory and recompile.

------- Usage -------

// Create module UObject
auto TorchModule = USimpleTorchModule::CreateSimpleTorchModule(this);

// Load script model
TorchModule->LoadTorchScriptModel(ModelFileFile);

// Fill input tensor
FSimpleTorchTensor Inputs = FSimpleTorchTensor( { 1, 2 } );
Inputs[0][0] = 1.f; Inputs[0][1] = 1.f;

// call model method
FSimpleTorchTensor Outputs = FSimpleTorchTensor( { 1 } );
TorchModule->ExecuteModelMethod("forward", Inputs, Outputs);

// read result
UE_LOG(LogTemp, Log, TEXT("Returned value: %f"), Outputs[0]);
