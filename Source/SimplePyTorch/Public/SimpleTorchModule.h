// VR IK Body Plugin
// (c) Yuri N Kalinin, 2021, ykasczc@gmail.com. All right reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimpleTorchModule.generated.h"

/*
* Simple struct to represent multidimensional float arrays
* It's not intended to process large data.
* Use it to store NN inputs and outputs.
* Don't create it every time. Create tensor object once and then set its data when needed.
*/
/*
* Simple struct to represent multidimensional float arrays
* It's not intended to process large data.
* Use it to store NN inputs and outputs.
* Don't create it every time. Create tensor object once and then set its data when needed.
*/
USTRUCT(BlueprintType)
struct SIMPLEPYTORCH_API FSimpleTorchTensor
{
	GENERATED_USTRUCT_BODY()

protected:
	float* Data;

	/** Data dimensions */
	UPROPERTY()
	TArray<int32> Dimensions;

	/** Is memory for Data allocated by this object? */
	UPROPERTY()
	bool bDataOwner;

	/** Internal cache is navigate through Data */
	UPROPERTY()
	TArray<int32> AddressMultipliersCache;

	/** Size of Data (size in bytes = sizeof(float) * DataSize */
	UPROPERTY()
	int32 DataSize;

	/** Object created Data (for bDataOwner == false) */
	FSimpleTorchTensor* ParentTensor;

	/** Other objects using Data (for bDataOwner == true) */
	TSet<FSimpleTorchTensor*> DataUsers;

	/** Initialize AddressMultipliersCache */
	void InitAddressSpace();

	/** Get flat address in Data from multidimensional address */
	int32 GetAddress(TSet<int32> Address) const;
public:

	FSimpleTorchTensor()
		: Data(NULL)
		, bDataOwner(true)
		, DataSize(0)
		, ParentTensor(nullptr)
	{}
	FSimpleTorchTensor(FSimpleTorchTensor* Parent, TSet<int32> SubAddress)
		: Data(NULL)
		, bDataOwner(true)
		, DataSize(0)
		, ParentTensor(nullptr)
	{
		CreateAsChild(Parent, SubAddress);
	}
	FSimpleTorchTensor(TSet<int32> Dimensions)
		: Data(NULL)
		, bDataOwner(true)
		, DataSize(0)
		, ParentTensor(nullptr)
	{
		Create(Dimensions);
	}
	~FSimpleTorchTensor()
	{
		Cleanup();
	}

	// Clear Data
	void Cleanup();

	// Set dimensions and allocate memory
	bool Create(TSet<int32> TensorDimensions);

	// Create tensor as a subtensor in another tensor (share the same memory)
	bool CreateAsChild(FSimpleTorchTensor* Parent, TSet<int32> Address);

	// Is tensor initialized?
	bool IsValid() const { return Data != NULL; }

	// Is tensor parent fro this data?
	bool IsDataOwner() const { return bDataOwner; }

	// Get current tensor dimensions
	TSet<int32> GetDimensions() const;

	// Get number of itemes in flat array
	int32 GetDataSize() const { return DataSize; }

	// Get first value (without offset) as float
	float Item();

	// Get raw data as points of [Size] size
	float* GetRawData(int32* Size) const;
	float* GetRawData() const { return Data; }

	// Convert multidimensional address to flat address
	int32 GetRawAddress(TSet<int32> Address) const { return GetAddress(Address); }

	// Get reference to single float value with address
	float* GetCell(TSet<int32> Address);
	float GetValue(TSet<int32> Address) const;

	/* Create float array. Only works for tensor with one dimension.
	* Ex: auto p = FSimpleTorchTensor({ 4, 12 });
	* p[2].ToArray(arr);
	*	(arr.Num() == 12)
	* p.ToArray(arr);
	*	no, p has two dimensions
	*/
	bool ToArray(TArray<float>& OutArray) const;

	/* Fill tensor from array. Only works for tensor with one dimension.
	* Ex. auto p = FSimpleTorchTensor({ 8, 2 });
	* TArray<float> InData; InData.Add(12.f); InData.Add(24.f);
	* p[7].FromArray(InData);
	*	correct, (p[7][0] == 12.f && p[7][1] == 24.f)
	* p.FromArray(InData);
	*	error: p has two dimensions
	*/
	bool FromArray(const TArray<float>& InData);

	// Change dimensions.
	// Only keeps data if new overall size is equal to old sizse
	bool Reshape(TSet<int32> NewShape);

	// Create copy of this tensor
	FSimpleTorchTensor Detach();

	// Apply scale and then offset only to first item of tensor
	void ScaleItem(float Scale, float Offset);

	FSimpleTorchTensor operator[](int32 SubElement);
	float operator=(const float& Value);
	bool operator==(const float& Value) const;
	FSimpleTorchTensor& operator*=(float Scale);
	FSimpleTorchTensor& operator+=(float Value);
	FSimpleTorchTensor& operator-=(float Value);
};

/**
 * 
 */
UCLASS(Blueprintable)
class SIMPLEPYTORCH_API USimpleTorchModule : public UObject
{
	GENERATED_BODY()
	
public:
	USimpleTorchModule();
	//~USimpleTorchModule();

	virtual void BeginDestroy() override;

	/** Create new controller from blueprint */
	UFUNCTION(BlueprintCallable, Category = "Simple Torch")
	static USimpleTorchModule* CreateSimpleTorchModule(UObject* InParent);

	/** Load neural net model from file */
	UFUNCTION(BlueprintCallable, Category = "Simple Torch")
	bool LoadTorchScriptModel(FString FileName);

	/** Is any neural net model loaded? */
	UFUNCTION(BlueprintPure, Category = "Simple Torch")
	bool IsTorchModelLoaded() const;

	/** Execute Forward(...) method in loaded model */
	UFUNCTION(BlueprintCallable, Category = "Simple Torch", meta = (DisplayName = "Do Forward Call"))
	bool DoForwardCall(const FSimpleTorchTensor& InData, FSimpleTorchTensor& OutData);

	/** Execute any method by name in loaded model */
	UFUNCTION(BlueprintCallable, Category = "Simple Torch", meta = (DisplayName = "Do Forward Call"))
	bool ExecuteModelMethod(const FString& MethodName, const FSimpleTorchTensor& InData, FSimpleTorchTensor& OutData);

private:
	/** Buffer size (default) */
	int32 BufferSize;

	/** Buffer to get results from torch method */
	float* Buffer;
	/** Output Buffer dimensions */
	int* BufferDims;
};
