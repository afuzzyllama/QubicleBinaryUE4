// Copyright 2015 afuzzyllama. All Rights Reserved.

#include "QubicleBinaryPrivatePCH.h"
#include "QubicleBinaryReader.h"

const uint32 QubicleBinaryReader::CODEFLAG = 2;
const uint32 QubicleBinaryReader::NEXTSLICEFLAG = 6;


bool QubicleBinaryReader::ReadIntoObject(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject)
{
    FMemoryReader Reader(BinaryData, false);
    Reader.Seek(0);
    
    uint8 Major, Minor, Release, Build;
    Reader << Major;
    Reader << Minor;
    Reader << Release;
    Reader << Build;

    UE_LOG(LogQubicleBinary, Verbose, TEXT("Version: %i.%i.%i.%i"), Major, Minor, Release, Build);
    
    // Version selection
    switch(Major)
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
                    return QubicleBinaryReader::ReadIntoObject_1_1_0_0(BinaryData, ContainerObject);
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

bool QubicleBinaryReader::ReadIntoObject_1_1_0_0(TArray<uint8>& BinaryData, IQubicleBinaryAccessor* ContainerObject)
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
    for (int32 i = 0; i < NumMatrices; ++i)
    {
        // read matrix name
        uint8 MatrixNameLength;
        FString MatrixName;
        
        // Read the length
        Reader << MatrixNameLength;
        
        // Get a pointer to the position in the BinaryData array that points to the beginning on the string.
        // Read the string into the variable
        MatrixName = FString::FromBlob(BinaryData.GetData() + Reader.Tell(), MatrixNameLength);
        
        // Seek the reader to after the string position
        Reader.Seek(Reader.Tell() + MatrixNameLength);
        
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
        
        ContainerObject->InitContainer(MatrixName, PosX, PosY, PosZ, SizeX, SizeY, SizeZ);
        
        if (Compressed == 0)
        {
            // if uncompressd
            for(uint32 z = 0; z < SizeZ; ++z)
            {
                for(uint32 y = 0; y < SizeY; y++)
                {
                    for(uint32 x = 0; x < SizeX; x++)
                    {
                        ContainerObject->SetVoxel(MatrixName, x, y, z, QubicleBinaryReader::ReadColor(&Reader, ColorFormat));
                    }
                }
            }
        }
        else
        {
            // if compressed
            
            uint32 Index;
            for(uint32 z = 0; z < SizeZ; ++z)
            {
                z++;
                Index = 0;
       
                uint32 Data;
                uint32 Count;
                while (true)
                {
                    Reader << Data;
         
                    uint32 x, y;
                    if (Data == QubicleBinaryReader::NEXTSLICEFLAG)
                    {
                        break;
                    }
                    else if (Data == QubicleBinaryReader::CODEFLAG)
                    {
                        Reader << Count;
                        Reader << Data;


                        for(uint32 j = 0; j < Count; ++j)
                        {
                            x = Index % SizeX + 1; // mod = modulo e.g. 12 mod 8 = 4
                            y = Index / SizeX + 1; // div = integer division e.g. 12 div 8 = 1
                            Index++;

                            ContainerObject->SetVoxel(MatrixName, x, y, z, QubicleBinaryReader::ReadColor(&Reader, ColorFormat));
                        }
                    }
                    else
                    {
                        x = Index % SizeX + 1;
                        y = Index / SizeX + 1;
                        Index++;

                        ContainerObject->SetVoxel(MatrixName, x, y, z, QubicleBinaryReader::ReadColor(&Reader, ColorFormat));
                    }
                }
            }
        }
    }
    
    return true;
}


FColor QubicleBinaryReader::ReadColor(FMemoryReader* Reader, uint32 ColorFormat)
{
    uint8 R,G,B,A;
    if(ColorFormat == 0)
    {
        // RGBA
        (*Reader) << R;
        (*Reader) << G;
        (*Reader) << B;
        (*Reader) << A;
    }
    else
    {
        // BGRA
        (*Reader) << B;
        (*Reader) << G;
        (*Reader) << R;
        (*Reader) << A;
    }
    
    // Ignoring A component because masking is done outside of loading
    return FColor(R,G,B);
}
