// Copyright 2015 afuzzyllama. All Rights Reserved.

#include "QubicleBinaryPrivatePCH.h"
#include <string>	// Convert std::string to FString
#include "QubicleBinaryReader.h"


const uint32 FQubicleBinaryReader::CODEFLAG = 2;
const uint32 FQubicleBinaryReader::NEXTSLICEFLAG = 6;

bool FQubicleBinaryReader::ReadFileIntoObject(FString FileName, IQubicleBinaryAccessor* ContainerObject)
{
	check(ContainerObject != nullptr);

	TArray<uint8> BinaryData;
	if (!FFileHelper::LoadFileToArray(BinaryData, *FileName))
	{
		UE_LOG(LogQubicleBinary, Error, TEXT("Could not read file"));
	}

	return FQubicleBinaryReader::ReadIntoObject(BinaryData, ContainerObject);

}

bool FQubicleBinaryReader::ReadIntoObject(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject)
{
	check(ContainerObject != nullptr);

	FMemoryReader Reader(BinaryData, false);
	Reader.Seek(0);

	uint8 Major, Minor, Release, Build;
	Reader << Major;
	Reader << Minor;
	Reader << Release;
	Reader << Build;

	UE_LOG(LogQubicleBinary, Verbose, TEXT("Version: %i.%i.%i.%i"), Major, Minor, Release, Build);

	// Version selection
	switch (Major)
	{
	case 1:
		switch (Minor)
		{
		case 1:
			switch (Release)
			{
			case 0:
				switch (Build)
				{
				case 0:
					return FQubicleBinaryReader::ReadIntoObject_1_1_0_0(BinaryData, ContainerObject);
					break;
				}
				break;
			}
		}
	}

	// error
	UE_LOG(LogQubicleBinary, Error, TEXT("Version not implemented: %i.%i.%i.%i"), Major, Minor, Release, Build);
	return false;
}

bool FQubicleBinaryReader::ReadIntoObject_1_1_0_0(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject)
{
	FMemoryReader Reader(BinaryData, false);

	// Skip reading the version which is uint32
	Reader.Seek(4);

	uint32 ColorFormat; // 0 = RGBA
	// 1 = BGRA

	uint32 ZAxisOrientation; // 0 = left handed
	// 1 = right handed

	uint32 Compressed; // 0 = uncompressed
	// 1 = compressed using run length encoding (RLE)

	uint32 VisibilityMaskEncoded;   // 0 = the A value of RGBA or BGRA is either 0 (invisble voxel) or 255 (visible voxel).
	// 1 = the visibility mask of each voxel is encoded into the A value telling your software which sides of the voxel are visible. You can save a lot of render time using this option. More info about this in the section visibility-mask encoding.
	// if (mask == 0) // voxel invisble
	// if (mask && 2 == 2) // left side visible
	// if (mask && 4 == 4) // right side visible
	// if (mask && 8 == 8) // top side visible
	// if (mask && 16 == 16) // bottom side visible
	// if (mask && 32 == 32) // front side visible
	// if (mask && 64 == 64) // back side visible

	uint32 NumMatrices;

	Reader << ColorFormat;
	Reader << ZAxisOrientation;
	Reader << Compressed;
	Reader << VisibilityMaskEncoded;
	Reader << NumMatrices;

	// for each matrix stored in file
	for (uint32 i = 0; i < NumMatrices; ++i)
	{
		// read matrix name
		uint8 MatrixNameLength;

		// Read the length
		Reader << MatrixNameLength;

		// Get a pointer to the position in the BinaryData array that points to the beginning on the string.
		// Read the string into the variable
		FString MatrixName("");
		for (uint8 CharNum = 0; CharNum < MatrixNameLength; ++CharNum)
		{
			uint8 CurrentByte;
			Reader << CurrentByte;

			FString CurrentChar = FString::Printf(TEXT("%c"), CurrentByte);
			MatrixName += CurrentChar;
		}


		// read matrix size
		uint32 SizeX, SizeY, SizeZ;
		Reader << SizeX;
		Reader << SizeY;
		Reader << SizeZ;

		// read matrix position (in this example the position is irrelevant)
		int32 PosX, PosY, PosZ;
		Reader << PosX;
		Reader << PosY;
		Reader << PosZ;

		ContainerObject->InitContainer(MatrixName, PosX, PosY, PosZ, SizeX, SizeZ, SizeY);

		uint32 Data;
		FColor NewColor;
		if (Compressed == 0)
		{
			// if uncompressd
			for (uint32 z = 0; z < SizeZ; ++z)
			{
				for (uint32 y = 0; y < SizeY; y++)
				{
					for (uint32 x = 0; x < SizeX; x++)
					{
						Reader << Data;

						NewColor = FQubicleBinaryReader::ReadColor(Data, ColorFormat);
						if (NewColor.A != 0)
						{
							ContainerObject->SetVoxel(MatrixName, x, z, y, NewColor);
						}
					}
				}
			}
		}
		else
		{
			// if compressed

			uint32 Index;
			for (uint32 z = 0; z < SizeZ; ++z)
			{
				Index = 0;

				uint32 Count;
				while (true)
				{
					Reader << Data;

					uint32 x, y;
					if (Data == FQubicleBinaryReader::NEXTSLICEFLAG)
					{
						break;
					}
					else if (Data == FQubicleBinaryReader::CODEFLAG)
					{
						Reader << Count;
						Reader << Data;


						for (uint32 j = 0; j < Count; ++j)
						{
							x = Index % SizeX; // mod = modulo e.g. 12 mod 8 = 4
							y = Index / SizeX; // div = integer division e.g. 12 div 8 = 1
							++Index;

							NewColor = FQubicleBinaryReader::ReadColor(Data, ColorFormat);
							if (NewColor.A != 0)
							{
								ContainerObject->SetVoxel(MatrixName, x, z, y, NewColor);
							}

						}
					}
					else
					{
						x = Index % SizeX;
						y = Index / SizeX;
						++Index;

						NewColor = FQubicleBinaryReader::ReadColor(Data, ColorFormat);
						if (NewColor.A != 0)
						{
							ContainerObject->SetVoxel(MatrixName, x, z, y, NewColor);
						}

					}
				}
			}
		}
	}

	return true;
}



FColor FQubicleBinaryReader::ReadColor(uint32 Color, uint32 ColorFormat)
{
    uint8 R,G,B,A;
	if (ColorFormat == 0)
	{
		// RGBA
		A = (Color & 0xFF000000) >> 24;
		B = (Color & 0x00FF0000) >> 16;
		G = (Color & 0x0000FF00) >> 8;
		R = (Color & 0x000000FF);
	}
	else
	{
		// BGRA
		A = (Color & 0xFF000000) >> 24;
		R = (Color & 0x00FF0000) >> 16;
		G = (Color & 0x0000FF00) >> 8;
		B = (Color & 0x000000FF);
	}
    
    // Ignoring A component because masking is done outside of loading
    return FColor(R,G,B,A);
}
