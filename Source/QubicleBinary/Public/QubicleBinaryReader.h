// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

/**
 * Interface to enable setting of voxels on a game asset
 */
class IQubicleBinaryAccessor
{
public:
    virtual ~IQubicleBinaryAccessor() {}

	/**
	  *	Initialize a "Matrix" from the Qubicle Editor.  Assumes that the initialized container is empty.
	  */
    virtual void InitContainer(FString ContainerName, int32 PosX, int32 PosY, int32 PosZ, uint32 SizeX, uint32 SizeY, uint32 SizeZ) = 0;

	/**
	 *	Sets a voxel inside the supplied container 
	 */
    virtual void SetVoxel(FString ContainerName, uint32 x, uint32 y, uint32 z, FColor Color) = 0;
};

class QUBICLEBINARY_API FQubicleBinaryReader
{
public:
	/**
	 *	Reads a Qubicle Binary file on to an object that implents the IQubicleBinaryAccessor interface
	 */
    static bool ReadFileIntoObject(FString FileName, IQubicleBinaryAccessor* ContainerObject);

	/**
	 *	Reads a byte array filled with a Qubicle Binary file on to an object that implents the IQubicleBinaryAccessor interface
	 */
    static bool ReadIntoObject(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject);
    
private:
    static const uint32 CODEFLAG;
    static const uint32 NEXTSLICEFLAG;

    static bool ReadIntoObject_1_1_0_0(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject);

	/**
	 * Converts a uint32 to FColor.  Format Qubicle Editor uses is different than Unreal's FColor(uint32) constructor, so that conversion is taken care of here.
	 */
	static FColor ReadColor(uint32 Color, uint32 ColorFormat);
};