// Copyright 2015 afuzzyllama. All Rights Reserved.
#pragma once

class IQubicleBinaryAccessor
{
public:
    virtual ~IQubicleBinaryAccessor() {}
    virtual void InitContainer(FString ContainerName, int32 PosX, int32 PosY, int32 PosZ, uint32 SizeX, uint32 SizeY, uint32 SizeZ) = 0;
    virtual void SetVoxel(FString ContainerName, uint32 x, uint32 y, uint32 z, FColor Color) = 0;
};

class FQubicleBinaryReader
{
public:
    static bool ReadFileIntoObject(FString FileName, IQubicleBinaryAccessor* ContainerObject);

    static bool ReadIntoObject(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject);
    
private:
    static const uint32 CODEFLAG;
    static const uint32 NEXTSLICEFLAG;

    static bool ReadIntoObject_1_1_0_0(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject);
    
    static FColor ReadColor(FMemoryReader* Reader, uint32 ColorFormat);
};