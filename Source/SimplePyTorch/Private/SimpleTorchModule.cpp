// VR IK Body Plugin
// (c) Yuri N Kalinin, 2021, ykasczc@gmail.com. All right reserved.

#include "SimpleTorchModule.h"
#include "SimplePyTorch.h"
#include "Modules/ModuleManager.h"
#include "HAL/UnrealMemory.h"
#include <vector>

USimpleTorchModule::USimpleTorchModule()
	: ModelId(INDEX_NONE)
{
	Buffer = NULL;
	BufferDims = NULL;
	BufferSize = 256;
}

void USimpleTorchModule::BeginDestroy()
{
	Super::BeginDestroy();
	if (Buffer != NULL)
	{
		delete[] Buffer;
		Buffer = NULL;
	}
	if (BufferDims != NULL)
	{
		delete[] BufferDims;
		BufferDims = NULL;
	}
}

USimpleTorchModule* USimpleTorchModule::CreateSimpleTorchModule(UObject* InParent)
{
	return NewObject<USimpleTorchModule>(InParent);
}

bool USimpleTorchModule::LoadTorchScriptModel(FString FileName)
{
	FSimplePyTorchModule& Module = FModuleManager::GetModuleChecked<FSimplePyTorchModule>(TEXT("SimplePyTorch"));

	bool bResult = false;
	if (Module.bDllLoaded)
	{
		if (Buffer != NULL)
		{
			delete[] Buffer;
			Buffer = NULL;
		}
		Buffer = new float[BufferSize];
		if (BufferDims != NULL)
		{
			delete[] BufferDims;
			BufferDims = NULL;
		}
		BufferDims = new int[16];

		ModelId = Module.FuncTSW_LoadScriptModel(TCHAR_TO_ANSI(*FileName));
		bResult = (ModelId != INDEX_NONE);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USimpleTorchModule: DLL not loaded"));
	}

	return bResult;
}

bool USimpleTorchModule::IsTorchModelLoaded() const
{
	FSimplePyTorchModule& Module = FModuleManager::GetModuleChecked<FSimplePyTorchModule>(TEXT("SimplePyTorch"));

	bool bResult = false;
	if (Module.bDllLoaded)
	{
		return Module.FuncTSW_CheckModel(ModelId);
	}

	return bResult;
}

bool USimpleTorchModule::DoForwardCall(const FSimpleTorchTensor& InData, FSimpleTorchTensor& OutData)
{
	return ExecuteModelMethod(TEXT("forward"), InData, OutData);
}

bool USimpleTorchModule::ExecuteModelMethod(const FString& MethodName, const FSimpleTorchTensor& InData, FSimpleTorchTensor& OutData)
{
	FSimplePyTorchModule& Module = FModuleManager::GetModuleChecked<FSimplePyTorchModule>(TEXT("SimplePyTorch"));
	
	bool bResult = false;
	if (Module.bDllLoaded && Buffer != NULL && OutData.IsDataOwner())
	{
		TArray<int> InDims = InData.GetDimensions();

		float* pOutData = OutData.IsValid()
			? OutData.GetRawData()
			: Buffer;

		int OutDimsCount = 0;
		if (MethodName == TEXT("forward"))
		{
			Module.FuncTSW_ForwardPass_Def(ModelId, InData.GetRawData(), InDims.GetData(), InDims.Num(),
				pOutData, BufferDims, &OutDimsCount);
		}
		else
		{
			Module.FuncTSW_Execute_Def(ModelId, TCHAR_TO_ANSI(*MethodName), InData.GetRawData(), InDims.GetData(), InDims.Num(),
				pOutData, BufferDims, &OutDimsCount);
		}

		bResult = (OutDimsCount > 0);
		if (bResult)
		{
			TArray<int32> OldOutDims = OutData.GetDimensions();
			bool bOutTensorMatches = (OutDimsCount == OutData.GetDimensions().Num());
			TArray<int32> NewOutDims;

			int32 Length = 1;
			for (int i = 0; i < OutDimsCount; i++)
			{
				Length *= BufferDims[i];
				NewOutDims.Add(BufferDims[i]);
				if (bOutTensorMatches && BufferDims[i] != OldOutDims[i])
				{
					bOutTensorMatches = false;
				}
			}

			if (bOutTensorMatches)
			{
				// do nothing
			}
			else
			{
				if (OutData.IsValid())
				{
					if (!OutData.Reshape(NewOutDims))
					{
						OutData.Cleanup();
						OutData.Create(NewOutDims);
					}
				}
				else
				{
					OutData.Create(NewOutDims);
				}
				FMemory::Memcpy(OutData.GetRawData(), Buffer, Length * sizeof(float));
			}
		}
	}

	return bResult;
}

/******************************************************************************************************/
/* FSimpleTorchTensor																					  */
/******************************************************************************************************/

void FSimpleTorchTensor::InitAddressSpace()
{
	AddressMultipliersCache.Empty();
	AddressMultipliersCache.AddUninitialized(Dimensions.Num());

	int32 CurrVal = 1;
	int32 Last = Dimensions.Num() - 1;
	AddressMultipliersCache[Last] = 1;
	for (int32 Dim = Last - 1; Dim != INDEX_NONE; Dim--)
	{
		CurrVal *= Dimensions[Dim + 1];
		AddressMultipliersCache[Dim] = CurrVal;
	}
}

void FSimpleTorchTensor::Cleanup()
{
	if (Data)
	{
		if (bDataOwner)
		{
			for (auto& Child : DataUsers)
			{
				if (Child) Child->Cleanup();
			}
			DataUsers.Empty();
			delete[] Data;
		}
		else
		{
			if (ParentTensor)
			{
				ParentTensor->DataUsers.Remove(this);
			}
		}
		Data = NULL;
		Dimensions.Empty();
	}
}

int32 FSimpleTorchTensor::GetAddress(TArray<int32> Address) const
{
	if (!Data)
	{		
		return INDEX_NONE;
	}

	TArray<int32> AddrArray = Address;
	int32 Addr = 0;
	for (int32 i = 0; i < AddressMultipliersCache.Num(); i++)
	{
		int32 n = AddrArray.IsValidIndex(i) ? AddrArray[i] : 0;
		Addr += AddressMultipliersCache[i] * n;

		if (Addr >= DataSize)
		{
			return INDEX_NONE;
		}
	}

	return Addr;
}

bool FSimpleTorchTensor::Create(TArray<int32> TensorDimensions)
{
	if (Data)
	{
		Cleanup();
	}

	if (TensorDimensions.Num() == 0) return false;

	DataSize = 1;
	for (const auto& Dim : TensorDimensions)
		DataSize *= Dim;

	if (DataSize == 0) return false;

	Dimensions = TensorDimensions;
	InitAddressSpace();

	Data = new float[DataSize];

	bDataOwner = true;
	return true;
}

bool FSimpleTorchTensor::CreateAsChild(FSimpleTorchTensor* Parent, TArray<int32> Address)
{
	bDataOwner = false;

	if (!Parent)
	{
		return false;
	}

	if (Address.Num() > Parent->Dimensions.Num())
	{
		UE_LOG(LogTemp, Log, TEXT("CreateAsChild: dimenstions error: Address (%d) vs Parent (%d)"), Address.Num(), Parent->Dimensions.Num());
		return false;
	}

	int32 Addr = Parent->GetAddress(Address);
	if (Addr == INDEX_NONE)
	{
		UE_LOG(LogTemp, Log, TEXT("CreateAsChild: parent has no address"));
		return false;
	}

	if (Address.Num() == Parent->Dimensions.Num())
	{
		Dimensions.Empty();
		Dimensions.Add(1);
	}
	else
	{
		Dimensions.Empty();
		for (int32 i = Address.Num(); i < Parent->Dimensions.Num(); i++)
		{
			Dimensions.Add(Parent->Dimensions[i]);
		}
	}

	DataSize = 1;
	for (const auto& Dim : Dimensions)
		DataSize *= Dim;

	Data = &(Parent->Data[Addr]);
	ParentTensor = Parent->bDataOwner ? Parent : Parent->ParentTensor;
	if (!ParentTensor)
	{
		return false;
	}

	ParentTensor->DataUsers.Add(this);

	InitAddressSpace();

	return true;
}

TArray<int32> FSimpleTorchTensor::GetDimensions() const
{
	TArray<int32> t;
	for (const auto& d : Dimensions)
		t.Add(d);

	return t;
}

float FSimpleTorchTensor::Item()
{
	return IsValid() ? Data[0] : 0.f;
}

float* FSimpleTorchTensor::GetRawData(int32* Size) const
{
	if (!Data)
	{
		return nullptr;
	}
	else
	{
		int32 s = 1;
		for (const auto& Dim : Dimensions)
			if (Dim != 0)
				s *= Dim;

		*Size = s;

		return Data;
	}
}

float* FSimpleTorchTensor::GetCell(TArray<int32> Address)
{
	int32 Addr = GetAddress(Address);
	return Addr == INDEX_NONE ? NULL : &Data[Addr];
}

float FSimpleTorchTensor::GetValue(TArray<int32> Address) const
{
	int32 Addr = GetAddress(Address);
	return Addr == INDEX_NONE ? 0 : Data[Addr];
}

bool FSimpleTorchTensor::ToArray(TArray<float>& OutArray) const
{
	if (Dimensions.Num() == 1)
	{
		int32 Num = Dimensions[0];		
		OutArray.SetNumUninitialized(Num);		
		FMemory::Memcpy(OutArray.GetData(), Data, Num * sizeof(float));

		return true;
	}

	return false;
}

bool FSimpleTorchTensor::FromArray(const TArray<float>& InData)
{
	if (Dimensions.Num() == 1)
	{
		int32 Num = Dimensions[0];

		if (InData.Num() <= Num)
		{
			FMemory::Memcpy(Data, InData.GetData(), InData.Num() * sizeof(float));
			return true;
		}
	}

	return false;
}

bool FSimpleTorchTensor::Reshape(TArray<int32> NewShape)
{
	if (NewShape.Num() == 0)
	{
		return false;
	}
	if (!Data)
	{
		return false;
	}

	int32 NewDataSize = 1;
	for (const auto& Dim : NewShape)
	{
		NewDataSize *= Dim;
	}

	if (NewDataSize == DataSize)
	{
		Dimensions = NewShape;
		InitAddressSpace();
	}
	else
	{
		if (bDataOwner && DataUsers.Num() == 0)
		{
			Cleanup();

			DataSize = NewDataSize;
			Dimensions = NewShape;
			Data = new float[DataSize];

			InitAddressSpace();
		}
		else
		{
			// don't reshape sub-tensors?
			return false;
		}
	}

	return true;
}

FSimpleTorchTensor FSimpleTorchTensor::Detach()
{
	if (!Data)
	{
		return FSimpleTorchTensor();
	}

	TArray<int32> Dims;
	for (const auto& Val : Dimensions) Dims.Add(Val);

	FSimpleTorchTensor ret = FSimpleTorchTensor(Dims);
	FMemory::Memcpy(ret.GetRawData(), Data, DataSize);

	return ret;
}

void FSimpleTorchTensor::ScaleItem(float Scale, float Offset)
{
	if (Data)
	{
		Data[0] = Data[0] * Scale + Offset;
	}
}

FSimpleTorchTensor FSimpleTorchTensor::operator[](int32 SubElement)
{
	if (!IsValid())
	{
		return *this;
	}
	if (SubElement < 0 || SubElement > Dimensions[0])
	{
		return *this;
	}
	
	FSimpleTorchTensor t = FSimpleTorchTensor(this, { SubElement });
	return (t.IsValid() ? t : *this);
}

float FSimpleTorchTensor::operator=(const float& Value)
{
	if (!Data)
	{
		return Value;
	}

	Data[0] = Value;
	return Data[0];
}

bool FSimpleTorchTensor::operator==(const float& Value) const
{
	if (!Data)
	{
		return false;
	}

	if (Dimensions.Num() != 1 || Dimensions[0] != 1)
	{
		return false;
	}

	return Data[0] == Value;
}

FSimpleTorchTensor& FSimpleTorchTensor::operator*=(float Scale)
{
	check(Data);
	check(Dimensions.Num() == 1 && Dimensions[0] == 1);
	
	Data[0] *= Scale;

	return *this;
}

FSimpleTorchTensor& FSimpleTorchTensor::operator+=(float Value)
{
	check(Data);
	check(Dimensions.Num() == 1 && Dimensions[0] == 1);

	Data[0] += Value;

	return *this;
}

FSimpleTorchTensor& FSimpleTorchTensor::operator-=(float Value)
{
	check(Data);
	check(Dimensions.Num() == 1 && Dimensions[0] == 1);

	Data[0] -= Value;

	return *this;
}